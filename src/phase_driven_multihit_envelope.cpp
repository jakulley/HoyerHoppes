#include "plugin.hpp"


struct Phase_driven_multihit_envelope : Module {
	enum ParamId {
		PHASE_MODE_PARAM,
		ATTACK_PARAM,
		SUSTAIN_PARAM,
		DECAY_PARAM,
		ATTACK_EXP_PARAM,
		DECAY_EXP_PARAM,
		HITS_PARAM,
		SKEW_PARAM,
		SPREAD_PARAM,
		PHASE_ENV_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RATE_INPUT,
		VCA_INPUT,
		ATTACK_INPUT,
		SUSTAIN_INPUT,
		DECAY_INPUT,
		SPACE_INPUT,
		HITS_INPUT,
		SKEW_INPUT,
		SPREAD_INPUT,
		PHASE_ENV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENV_OUTPUT,
		VCA_OUTPUT,
		TRIG_OUTPUT,
		PHASE_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		PHASE_MODE_LIGHT,
		ATTACK_LIGHT,
		SUSTAIN_LIGHT,
		DECAY_LIGHT,
		LIGHTS_LEN
	};
	int hitPattern = 0;
	Phase_driven_multihit_envelope() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(PHASE_MODE_PARAM, 0.f, 1.f, 1.f, "if off, accepts a clock pulse instead of phase");
		configParam(ATTACK_PARAM, 0.01f, 1.f, 0.01f, "attack time", "%", 0.f, 100.f);
		configParam(SUSTAIN_PARAM, 0.f, 1.f, 0.f, "sustain time", "%", 0.f, 100.f);
		configParam(DECAY_PARAM, 0.01f, 1.f, 0.5f, "decay time", "%", 0.f, 100.f);
		configParam(ATTACK_EXP_PARAM, 0.f, 2.f, 1.f, "");
		configParam(DECAY_EXP_PARAM, 0.f, 2.f, 1.f, "");
		configSwitch(HITS_PARAM, 1.f, 16.f, 1.f, "number of hits", {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"});
		configParam(SKEW_PARAM, 0.f, 1.f, 1.f, "");
		configParam(SPREAD_PARAM, 0.f, 2.1f, 1.f, "");
		configParam(PHASE_ENV_PARAM, 0.f, 1.f, 0.f, "envelope amplitude is limited by phase", "%", 0.f, 100.f);
		configInput(RATE_INPUT, "expects 0-10v");
		configInput(VCA_INPUT, "vca modulated by envelope");
		configInput(ATTACK_INPUT, "sums with attack parameter");
		configInput(SUSTAIN_INPUT, "sums with sustain parameter");
		configInput(DECAY_INPUT, "sums with decay parameter");
		configInput(HITS_INPUT, "bypasses hits parameter");
		configInput(SKEW_INPUT, "");
		configInput(SPREAD_INPUT, "");
		configInput(PHASE_ENV_INPUT, "attenuated by phase->env knob");
		configOutput(ENV_OUTPUT, "");
		configOutput(VCA_OUTPUT, "");
		configOutput(TRIG_OUTPUT, "");
		configOutput(PHASE_OUTPUT, "");
		configLight(PHASE_MODE_LIGHT, "");
		configLight(ATTACK_LIGHT, "");
		configLight(SUSTAIN_LIGHT, "");
		configLight(DECAY_LIGHT, "");
	}

	//clocking variables for !phase mode
	dsp::SchmittTrigger rateTrigger;
	float currentTime = 0.f;
	float period = 0.f;
	float lastTrigTime = 0.f;
	float rate = 0.f;
	//phase variables
	float phase = 0.f;
	float phaseOffset = 0.f;
	float skewValue = 0.f;
	float skewPhase = 0.f;
	float spreadValue = 0.f;
	float spreadCorrectly = 0.f;
	float spreadPhase = 0.f;

	float hit = 0.f;
	float hitReturn = 0.f;
	float hitPhase = 0.f;

	//envelope variables
	float attack = 0.f;
	float attackExp = 0.f;
	float attackExpCorrect = 0.f;
	float sustain = 0.f;
	float decay = 0.f;
	float decayExp = 0.f;
	float decayExpCorrect = 0.f;
	float alpha = 0.05f;  // Smoothing factor
	float smoothEnv = 0.f;

	//other utilities
	dsp::PulseGenerator trig;
	float VcaIn = 0.f;
	float env = 0.f;
	float amp = 0.f;

	void smoothEnvelope(float env) {
    	smoothEnv = alpha * env + (1.0f - alpha) * smoothEnv;
	}


	void process(const ProcessArgs& args) override {
		//set lights
		getLight(PHASE_MODE_LIGHT).setBrightness(getParam(PHASE_MODE_PARAM).getValue() == 1.f? 1.0 : 0.0);
		getLight(ATTACK_LIGHT).setBrightness(1.0);
		getLight(SUSTAIN_LIGHT).setBrightness(1.0);
		getLight(DECAY_LIGHT).setBrightness(1.0);
		//set phase
		if (getParam(PHASE_MODE_PARAM).getValue() == 1.) {
			phase = clamp(getInput(RATE_INPUT).getVoltage()/10 + phaseOffset);
		} else if(rateTrigger.process(getInput(RATE_INPUT).getVoltage())) {
			currentTime = args.frame;
			period = currentTime - lastTrigTime;
			lastTrigTime = currentTime;
			rate = args.sampleRate/period;
			phase = 0.f;
		}
		phase += rate*args.sampleTime;
		if (phase >= 1.) {
			phase -= 1.;
		}
		//skew the phase
		skewValue = clamp((getInput(SKEW_INPUT).isConnected()? getInput(SKEW_INPUT).getVoltage()/10 : getParam(SKEW_PARAM).getValue()), 0.001, 1.0);
		skewPhase = skewValue > phase ? phase/skewValue : (1-phase)/(1-skewValue);
		//spread the skewPhase - implemented as exponentiation
		spreadValue = clamp((getInput(SPREAD_INPUT).isConnected()? getInput(SPREAD_INPUT).getVoltage()/10 : getParam(SPREAD_PARAM).getValue()), 0.001, 2.0);
		if (spreadValue > 1.) {
			spreadCorrectly = pow(spreadValue, 3.323);
		} else {
			spreadCorrectly = spreadValue;
		}
		spreadPhase = pow(skewPhase, spreadCorrectly);
		//phase is set

		//multiply the phase by hits
		switch (hitPattern) {
			case 0:
				hit = getInput(HITS_INPUT).isConnected()? clamp(int((getInput(HITS_INPUT).getVoltage()/10)*16), 1., 16.) : getParam(HITS_PARAM).getValue();
				hitReturn = hit;
				break;
			case 1:
				hit = getInput(HITS_INPUT).isConnected()? clamp(int((getInput(HITS_INPUT).getVoltage()/10)*5), 1., 5.) : getParam(HITS_PARAM).getValue();
				hitReturn = powf(2, hit-1);
				break;
			case 2:
				hit = getInput(HITS_INPUT).isConnected()? clamp(int((getInput(HITS_INPUT).getVoltage()/10)*7), 1., 7.) : getParam(HITS_PARAM).getValue();
				hitReturn = 2*hit;
				break;
			case 3:
				hit = getInput(HITS_INPUT).isConnected()? clamp(int((getInput(HITS_INPUT).getVoltage()/10)*5), 1., 5.) : getParam(HITS_PARAM).getValue();
				hitReturn = 3*hit;
				break;
		}
		
		hitPhase = spreadPhase * hitReturn;
		while (hitPhase >= 1.) {
			hitPhase -= int(hitPhase);
		}
		
		//calculate envelope timings
		attack = getInput(ATTACK_INPUT).getVoltage()/10 + getParam(ATTACK_PARAM).getValue();
		attackExp = clamp(getParam(ATTACK_EXP_PARAM).getValue(), 0.001, 2.0);
		if (attackExp > 1.) {
			attackExpCorrect = pow(attackExp, 3.323);
		} else {
			attackExpCorrect = attackExp;
		}

		sustain = getInput(SUSTAIN_INPUT).getVoltage()/10 + getParam(SUSTAIN_PARAM).getValue();

		decay = getInput(DECAY_INPUT).getVoltage()/10 + getParam(DECAY_PARAM).getValue();
		decayExp = clamp(getParam(DECAY_EXP_PARAM).getValue(), 0.001, 2.0);
		if (decayExp > 1.) {
			decayExpCorrect = pow(decayExp, 3.323);
		} else {
			decayExpCorrect = decayExp;
		}

		//calculate envelope
		if (hitPhase < attack) {
			env = pow((hitPhase/attack), attackExpCorrect);
			trig.trigger();
		} else if (hitPhase < sustain) {
			env = 1.;
		} else if (hitPhase < decay) {
			env = pow((1+((hitPhase-(attack>sustain?attack:sustain))/((attack>sustain?attack:sustain)-decay))), decayExpCorrect);
		} else {
			env = 0.f;
			
		}
		amp = getInput(PHASE_ENV_INPUT).isConnected() ? (getInput(PHASE_ENV_INPUT).getVoltage()/10) * getParam(PHASE_ENV_PARAM).getValue() : getParam(PHASE_ENV_PARAM).getValue();
		env *= 1-(spreadPhase * amp);
		smoothEnvelope(env);

		

		bool trigProcess = trig.process(args.sampleTime);
		//set outputs
		getOutput(VCA_OUTPUT).setVoltage(getInput(VCA_INPUT).getVoltage()*smoothEnv);
		getOutput(TRIG_OUTPUT).setVoltage(trigProcess?10.f:0.f);
		getOutput(ENV_OUTPUT).setVoltage(10.f * smoothEnv);
		getOutput(PHASE_OUTPUT).setVoltage(10.f * spreadPhase);
	}

	json_t* dataToJson() override {
        json_t* rootJ = json_object();
        
        json_t* hitP = json_integer(hitPattern);
        json_object_set_new(rootJ, "hitPattern", hitP);

        return rootJ;
    }
    void dataFromJson(json_t* rootJ) override {

        // Load the activeRatios vector from JSON
        json_t* hitP = json_object_get(rootJ, "hitPattern");
        if (hitP) {
            hitPattern = json_integer_value(hitP);
        }
    }
};

struct EnvelopeDisplayWidget : public rack::OpaqueWidget {
public:
    Phase_driven_multihit_envelope *module = nullptr;

    void setModule(Phase_driven_multihit_envelope *module) {
        this->module = module;
    }

    EnvelopeDisplayWidget() {
        box.size = Vec(120, 60);
    }
	
	struct xy {
		float x;
		float y;
	};

	xy phaseCalculator(float boxSkew, float boxSpread, float t) {
		xy result;
		float x;
		float y;
		float peak = box.size.x * boxSkew;
		x = t * box.size.x;
		if (x <= peak) {
			float normalX = x / peak;
			float spread = pow(normalX, boxSpread);
			y = box.size.y - (box.size.y * spread);
		} else {
			float normalX = (x - peak) / (box.size.x - peak);
			float reverseSpread = pow(normalX, 1/boxSpread);
			y = box.size.y - (box.size.y * (1-reverseSpread)); 
		}
		result.x = x;
		result.y = y;
		return result;
	};

    void draw(const DrawArgs &args) override {
        if (!module) {
			// Draw the background
        nvgBeginPath(args.vg);
        nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(args.vg, nvgRGB(60, 60, 60));
        nvgFill(args.vg);
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, 0, box.size.y);
		nvgLineTo(args.vg, box.size.x, 0);
		nvgLineTo(args.vg, box.size.x, box.size.y);
		nvgFillColor(args.vg, nvgRGB(80,80,80));
		nvgFill(args.vg);
            return;
        }

		// Draw the background
        nvgBeginPath(args.vg);
        nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(args.vg, nvgRGB(60, 60, 60));
        nvgFill(args.vg);

		//new paradigm
		float currentPosition = module->phase;
		float skew = module->skewValue;
		float spread = module->spreadCorrectly;
		float hit = module->hitReturn;
		float hitWindow = box.size.y/hit;
		float hitWindowInc = 0.f;
		float y = 0.f;
		float attack = module->attack;
		float sustain = module->sustain;
		float decay = module->decay;
		//phase drawing
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, 0, box.size.y);
		for (int i = 0; i<box.size.x; i++) {
			float t = i/box.size.x;
			xy phase = phaseCalculator(skew, spread, t);
			if (i == 0) {
				nvgMoveTo(args.vg, phase.x, phase.y);
		 	} else {
		 		nvgLineTo(args.vg, phase.x, phase.y);
		 	}
		}
		nvgLineTo(args.vg, box.size.x, box.size.y);
		nvgFillColor(args.vg, nvgRGB(80,80,80));
		nvgFill(args.vg);

		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, 0, box.size.y);
		for (int i = 0; i<500.f; i++) {
			float t = i/500.f;
			xy phase = phaseCalculator(skew, spread, t);
			phase.y = box.size.y - phase.y;
			float amp = phase.y * module->amp;
			if (phase.x < box.size.x * skew) {
				if (phase.y-hitWindowInc < hitWindow) {
					if (phase.y-hitWindowInc < hitWindow*attack) {
						float normalY = ((phase.y-hitWindowInc)/(attack*hitWindow)*hit);
						y = box.size.y - (normalY*hitWindow);
					} else if (phase.y-hitWindowInc < hitWindow*sustain) {
						y = 0;
					} else if (phase.y-hitWindowInc < hitWindow*decay) {
						float normalY = ((phase.y-(hitWindow*(attack>sustain?attack:attack+(sustain-attack))))-hitWindowInc)/(((decay-(attack>sustain?attack:attack+(sustain-attack)))*hitWindow)/hit);
						y = (normalY*hitWindow);
					} else {
						y = box.size.y;
					}
				} else {
					hitWindowInc += hitWindow;
				}
			} else {
				if (phase.y-hitWindowInc < hitWindow) {
					if (phase.y-hitWindowInc < hitWindow*attack) {
						float normalY = ((phase.y-hitWindowInc)/(attack*hitWindow)*hit);
						y = box.size.y - (normalY*hitWindow);
						if (y >= box.size.y) {
							hitWindowInc -= hitWindow;
						}
					} else if (phase.y-hitWindowInc < hitWindow*sustain) {
						y = 0;
					} else if (phase.y-hitWindowInc < hitWindow*decay) {
						float normalY = ((phase.y-(hitWindow*(attack>sustain?attack:attack+(sustain-attack))))-hitWindowInc)/(((decay-(attack>sustain?attack:attack+(sustain-attack)))*hitWindow)/hit);
						y = (normalY*hitWindow);
					} else {
						y = box.size.y;
					}
				}
			}
			y = clamp(y, 0.f+amp, box.size.y);
			if (i == 0) {
				nvgMoveTo(args.vg, phase.x, box.size.y);
		 	} else {
		 		nvgLineTo(args.vg, phase.x, y);
		 	}
		}
		nvgStrokeColor(args.vg, nvgRGB(255,255,255));
		nvgStrokeWidth(args.vg, 0.75);
		nvgStroke(args.vg);

		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, box.size.x*currentPosition, box.size.y);
		nvgLineTo(args.vg, box.size.x*currentPosition, 0);
		nvgStrokeColor(args.vg, nvgRGB(200,200,200));
		nvgStrokeWidth(args.vg, 0.75);
		nvgStroke(args.vg);
    }
};

struct Phase_driven_multihit_envelopeWidget : ModuleWidget {
	void appendContextMenu(Menu *menu) override {
		Phase_driven_multihit_envelope *module = static_cast<Phase_driven_multihit_envelope*>(this->module);
		assert(module);
	
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Hit Count"));
			menu->addChild(createMenuItem("1-16", "default",
			[=]()	{
				module->configSwitch(module->HITS_PARAM, 1.f, 16.f, 1.f, "number of hits", {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"});
				module->hitPattern=0;
			}));
			menu->addChild(createMenuItem("1 and powers of two", "",
				[=]()	{
					module->configSwitch(module->HITS_PARAM, 1.f, 5.f, 1.f, "number of hits", {"1", "2", "4", "8", "16"});
					module->hitPattern=1;
				}));
			menu->addChild(createMenuItem("evens", "",
				[=]()	{
					module->configSwitch(module->HITS_PARAM, 1.f, 7.f, 1.f, "number of hits", {"2", "4", "6", "8", "10", "12", "16"});
					module->hitPattern=2;
				}));
			menu->addChild(createMenuItem("multiples of 3", "", 
				[=]()	{
					module->configSwitch(module->HITS_PARAM, 1.f, 5.f, 1.f, "number of hits", {"3", "6", "9", "12", "15"});
					module->hitPattern=3;
				}));
		menu->addChild(createMenuLabel("rate input range"));
			menu->addChild(createMenuItem("unipolar (0-10v)", "default",
				[=]()	{
					//default mode
					module->phaseOffset = 0.f;
				}));
			menu->addChild(createMenuItem("bipolar(-5-5v)", "",
				[=]()	{
					module->phaseOffset = 0.5f;
				}));
	}

	
	

	Phase_driven_multihit_envelopeWidget(Phase_driven_multihit_envelope* module) {
		setModule(module);
		setPanel(createPanel(
			asset::plugin(pluginInstance, "res/phase_driven_multihit_envelope.svg"),
			asset::plugin(pluginInstance, "res/phase_driven_multihit_envelope_dark_theme.svg")
		));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.6, 37.7)), module, Phase_driven_multihit_envelope::PHASE_MODE_PARAM, Phase_driven_multihit_envelope::PHASE_MODE_LIGHT));
		addParam(createLightParamCentered<VCVLightSlider<RedLight>>(mm2px(Vec(13.9, 60)), module, Phase_driven_multihit_envelope::ATTACK_PARAM, Phase_driven_multihit_envelope::ATTACK_LIGHT));
		addParam(createLightParamCentered<VCVLightSlider<RedLight>>(mm2px(Vec(27, 60)), module, Phase_driven_multihit_envelope::SUSTAIN_PARAM, Phase_driven_multihit_envelope::SUSTAIN_LIGHT));
		addParam(createLightParamCentered<VCVLightSlider<RedLight>>(mm2px(Vec(40.36, 60)), module, Phase_driven_multihit_envelope::DECAY_PARAM, Phase_driven_multihit_envelope::DECAY_LIGHT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(8.73, 68.115)), module, Phase_driven_multihit_envelope::ATTACK_EXP_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(35.35, 68.115)), module, Phase_driven_multihit_envelope::DECAY_EXP_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.389, 87.806)), module, Phase_driven_multihit_envelope::HITS_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.389, 100.98)), module, Phase_driven_multihit_envelope::SKEW_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.389, 114.153)), module, Phase_driven_multihit_envelope::SPREAD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(32.65, 88.6)), module, Phase_driven_multihit_envelope::PHASE_ENV_PARAM));


		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(11.9, 37.7)), module, Phase_driven_multihit_envelope::RATE_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(39.8, 37.7)), module, Phase_driven_multihit_envelope::VCA_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(11.9, 77.976)), module, Phase_driven_multihit_envelope::ATTACK_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(25.22, 77.976)), module, Phase_driven_multihit_envelope::SUSTAIN_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(38.584, 77.976)), module, Phase_driven_multihit_envelope::DECAY_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.903, 88.785)), module, Phase_driven_multihit_envelope::HITS_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.903, 101.958)), module, Phase_driven_multihit_envelope::SKEW_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.903, 115.132)), module, Phase_driven_multihit_envelope::SPREAD_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(42.8, 88)), module, Phase_driven_multihit_envelope::PHASE_ENV_INPUT));


		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(32.609, 103.19)), module, Phase_driven_multihit_envelope::ENV_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(42.721, 103.19)), module, Phase_driven_multihit_envelope::VCA_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(32.56, 116.87)), module, Phase_driven_multihit_envelope::TRIG_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(43.397, 116.87)), module, Phase_driven_multihit_envelope::PHASE_OUTPUT));

		
		EnvelopeDisplayWidget* envelopeDisplayWidget = createWidget<EnvelopeDisplayWidget>(mm2px(Vec(5, 10)));
		envelopeDisplayWidget->setModule(module);
		addChild(envelopeDisplayWidget);
	}
};


Model* modelPhase_driven_multihit_envelope = createModel<Phase_driven_multihit_envelope, Phase_driven_multihit_envelopeWidget>("phase_driven_multihit_envelope");