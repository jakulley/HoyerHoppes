#include "plugin.hpp"
using simd::float_4;
using simd::int32_4;

struct Scanning_frequency_division_osc_poly : Module {
	enum ParamId {
		FREQ_PARAM,
		SCAN_PARAM,
		WAVE_SHAPE_PARAM,
		FM_DEPTH_PARAM,
		SYNC_WINDOW_PARAM,
		WINDOWING_ON_PARAM,
		BY_1_PARAM,
		BY_2_PARAM,
		BY_3_PARAM,
		BY_4_PARAM,
		BY_5_PARAM,
		BY_6_PARAM,
		BY_7_PARAM,
		BY_8_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SCAN_CV_INPUT,
		VOCT_INPUT,
		WAVE_SHAPE_INPUT,
		FM_INPUT,
		FM_DEPTH_INPUT,
		SYNC_INPUT,
		BY_1_INPUT,
		BY_2_INPUT,
		BY_3_INPUT,
		BY_4_INPUT,
		BY_5_INPUT,
		BY_6_INPUT,
		BY_7_INPUT,
		BY_8_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		FREQ_OUTPUT,
		SCAN_OUTPUT,
		REFL_OUTPUT,
		LOW_OUTPUT,
		TRIG_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		WINDOWING_ON_LIGHT,
		LIGHTS_LEN
	};

	Scanning_frequency_division_osc_poly() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(FREQ_PARAM, -5.f, 5.f, 0.f, "");
		configParam(SCAN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(WAVE_SHAPE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FM_DEPTH_PARAM, -2.f, 2.f, 0.f, "");
		configParam(SYNC_WINDOW_PARAM, 0.f, 1.f, 0.f, "controls depth of windowing effect");
		configSwitch(WINDOWING_ON_PARAM, 0.f, 1.f, 0.f, "enable window for smooth sync");
		configSwitch(BY_1_PARAM, 0.f, 1.f, 0.f, "");
		configSwitch(BY_2_PARAM, 0.f, 1.f, 0.f, "");
		configSwitch(BY_3_PARAM, 0.f, 1.f, 0.f, "");
		configSwitch(BY_4_PARAM, 0.f, 1.f, 0.f, "");
		configSwitch(BY_5_PARAM, 0.f, 1.f, 0.f, "");
		configSwitch(BY_6_PARAM, 0.f, 1.f, 0.f, "");
		configSwitch(BY_7_PARAM, 0.f, 1.f, 0.f, "");
		configSwitch(BY_8_PARAM, 0.f, 1.f, 0.f, "");
		configInput(SCAN_CV_INPUT, "");
		configInput(VOCT_INPUT, "");
		configInput(WAVE_SHAPE_INPUT, "use negative voltage to subtract from the knob position");
		configInput(BY_1_INPUT, "");
		configInput(BY_2_INPUT, "");
		configInput(FM_INPUT, "");
		configInput(FM_DEPTH_INPUT, "");
		configInput(BY_3_INPUT, "");
		configInput(BY_4_INPUT, "");
		configInput(SYNC_INPUT, "");
		configInput(BY_5_INPUT, "");
		configInput(BY_6_INPUT, "");
		configInput(BY_7_INPUT, "");
		configInput(BY_8_INPUT, "");
		configOutput(FREQ_OUTPUT, "");
		configOutput(SCAN_OUTPUT, "");
		configOutput(REFL_OUTPUT, "");
		configOutput(LOW_OUTPUT, "");
		configOutput(TRIG_OUTPUT, "");
		configLight(WINDOWING_ON_LIGHT, "");
	}

	dsp::SchmittTrigger buttons[8];
	dsp::SchmittTrigger inputs[8];
	std::vector<int> activeRatios;
	dsp::PulseGenerator scanPulse;
	dsp::SchmittTrigger scanTrigger;
	
    int allDivisors[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
	float waveBlend = 0.f;
    
    struct Engine {
		dsp::SchmittTrigger syncCheck[4];
		dsp::PulseGenerator syncPulse[4];
		int32_4 currentScan = {0, 0, 0, 0};
    	int32_4 previousScan = {0, 0, 0, 0};
		float_4 scanValue = {0.f, 0.f, 0.f, 0.f};
    	float_4 clampScan = {0.f, 0.f, 0.f, 0.f};
    	float_4 reflectedScan = {0.f, 0.f, 0.f, 0.f};
    	float_4 fMod = {0.f, 0.f, 0.f, 0.f};
    	float_4 modIndex = {0.f, 0.f, 0.f, 0.f};
        float_4 phase = {0.f, 0.f, 0.f, 0.f};
        float_4 scanPhase = {0.f, 0.f, 0.f, 0.f};
        float_4 reflectedScanPhase = {0.f, 0.f, 0.f, 0.f};
        float_4 lowPhase = {0.f, 0.f, 0.f, 0.f};
        float_4 pitch = {0.f, 0.f, 0.f, 0.f};
        float_4 freq = {0.f, 0.f, 0.f, 0.f};
        float_4 fmFreq = {0.f, 0.f, 0.f, 0.f};
        float_4 scanFreq = {0.f, 0.f, 0.f, 0.f};
        float_4 reflectFreq = {0.f, 0.f, 0.f, 0.f};
        float_4 lowFreq = {0.f, 0.f, 0.f, 0.f};
    };
    Engine engines[4];

	int32_4 scanReturn(const int allDivisors[8], const std::vector<int>& activeRatios, const float_4& scanValue) {
		if (activeRatios.size() != 0) {
			//divide the scan range into equal sections according to the number of active ratios
			float scanInc = 1.f / (float)activeRatios.size();
			float_4 scanReturn;
			
			long long unsigned int i = 0;
			for (int j = 0; j<4; j++) {
				if (scanValue[j] < scanInc) {
					return allDivisors[activeRatios[i]];
				} else {
					while (scanValue[j] >= scanInc * (i+1) && i < activeRatios.size()-1) {
						i++;
					}
					scanReturn[j] = allDivisors[activeRatios[i]];
				}
			}
			return scanReturn;
		} else {
			return 1;
		}
	}

	float_4 chebyshevSin(float_4 x) {
		// Ensure the input is within the range [-pi, pi]
		x = simd::fmod(x, 2 * M_PI); // Wrap the input to be within [-2*pi, 2*pi]
		for (int i=0; i<4; i++) {
			if (x[i] > M_PI) x[i] -= 2 * M_PI;
			if (x[i] < -M_PI) x[i] += 2 * M_PI;
		}
	
		// Chebyshev polynomial approximation for sin(x)
		float_4 x2 = x * x;
		float_4 x3 = x2 * x;
		float_4 x5 = x3 * x2;
		float_4 x7 = x5 * x2;
		float_4 x9 = x7 * x2;
		float_4 x11 = x9 * x2;
		float_4 x13 = x11 * x2;
		float_4 x15 = x13 * x2;

		// 15th-degree Chebyshev approximation for sin(x)
		return x - (x3 / 6.0f) + (x5 / 120.0f) - (x7 / 5040.0f) + (x9 / 362880.0f) 
           - (x11 / 39916800.0f) + (x13 / 6227020800.0f) - (x15 / 1307674368000.0f);
	}

	float_4 triangleWave(float_4 x) {
		// Normalize x to the range [-pi, pi]
		x = simd::fmod(x, 2 * M_PI); // Wrap the input to be within [-2*pi, 2*pi]
		for (int i = 0; i < 4; i++) {
			if (x[i] > M_PI) x[i] -= 2 * M_PI;
			if (x[i] < -M_PI) x[i] += 2 * M_PI;
		}
		
		// Triangle wave formula: 
		// If x is in the range [0, pi], it linearly increases; if in the range [-pi, 0], it linearly decreases.
		return 2.0f * (simd::abs(x) / M_PI) - 1.0f;
	}
	
	float_4 sawWave(float_4 x) {
		// Normalize x to the range [-pi, pi]
		x = simd::fmod(x, 2 * M_PI); // Wrap the input to be within [-2*pi, 2*pi]
		for (int i = 0; i < 4; i++) {
			if (x[i] > M_PI) x[i] -= 2 * M_PI;
			if (x[i] < -M_PI) x[i] += 2 * M_PI;
		}
	
		// Sawtooth wave formula: x / pi, where x is within [-pi, pi]
		return (x / M_PI);
	}

	float_4 squareWave(float_4 x) {
		// Normalize x to the range [-pi, pi]
		x = simd::fmod(x, 2 * M_PI); // Wrap the input to be within [-2*pi, 2*pi]
		for (int i = 0; i < 4; i++) {
			if (x[i] > M_PI) x[i] -= 2 * M_PI;
			if (x[i] < -M_PI) x[i] += 2 * M_PI;
		}
	
		// Generate sine wave
		float_4 sine = simd::sin(x);
	
		// Manually create a mask (1 for positive sine, 0 for negative sine)
		int32_4 mask = simd::int32_4(0);
		for (int i = 0; i < 4; i++) {
			if (sine[i] > 0.0f) {
				mask[i] = 1;
			} else {
				mask[i] = -1;
			}
		}
	
		// Convert the mask to float_4 and return the result
		return simd::float_4(mask[0], mask[1], mask[2], mask[3]);
	}
	
	
		

	float_4 waveReturn(const float& waveBlend, float_4 phase, float_4 syncWindow) {
    	// compute common values that don’t depend on waveBlend
    	float windowDepth = getParam(SYNC_WINDOW_PARAM).getValue();
    	float window = 1.f - (getInput(SYNC_INPUT).getVoltage() / 5.f) * windowDepth;
		float_4 wave;
		float_4 sine = chebyshevSin(2.f * phase * M_PI);
		float_4 triangle = triangleWave(2.f * phase * M_PI);
		float_4 square = squareWave(2.f * phase * M_PI);
		float_4 saw = sawWave(2.f * phase * M_PI);
    	
    	// Compute the wave based on waveBlend
		if (waveBlend < 0.33333f) {
			wave = 3.f * ((sine * (0.33333f - waveBlend)) + (triangle * waveBlend));
		} else if (waveBlend < 0.666666f) {
			wave = 3.f * (triangle * (0.66666f - waveBlend) + saw * (waveBlend - 0.33333f));
		} else {
			wave = 3.f * (saw * (1.f - waveBlend) + square * (waveBlend - 0.66666f));
		}
		if (getInput(SYNC_INPUT).isConnected() && getParam(WINDOWING_ON_PARAM).getValue() == 1.f) {
			wave *= syncWindow * (window - windowDepth);
		}
    	return wave;
	}

	void process(const ProcessArgs& args) override {
		activeRatios.reserve(8);
		for (int i = 0; i < 8; i++) {
			rack::dsp::SchmittTrigger::Event buttonPress = buttons[i].processEvent(getParam(ParamId(i+6)).getValue(), 0.1f, 0.9f);
			rack::dsp::SchmittTrigger::Event inputActivate = inputs[i].processEvent(getInput(InputId(i+6)).getVoltage(), 0.1f, 0.9f);

			if ((buttonPress == 1 && (!inputs[i].isHigh())) || ((!buttons[i].isHigh()) && inputActivate == 1)) {
				activeRatios.push_back(i);
				sort(activeRatios.begin(), activeRatios.end());
			}
			if ((buttonPress == -1 && (!inputs[i].isHigh())) || (inputActivate == -1 && (!buttons[i].isHigh()))) {
      			activeRatios.erase(std::remove(activeRatios.begin(), activeRatios.end(), i), activeRatios.end());
				sort(activeRatios.begin(), activeRatios.end());
			}
		}

		waveBlend = getInput(WAVE_SHAPE_INPUT).isConnected()? clamp(((getInput(WAVE_SHAPE_INPUT).getVoltage()/10.f) + getParam(WAVE_SHAPE_PARAM).getValue())) : getParam(WAVE_SHAPE_PARAM).getValue();
		
        if (getParam(WINDOWING_ON_PARAM).getValue() == 1.f) {
            getLight(WINDOWING_ON_LIGHT).setBrightness(1.f);
        } else {
            getLight(WINDOWING_ON_LIGHT).setBrightness(0.f);
        }

		int channels = std::max(1, getInput(VOCT_INPUT).getChannels());
		for (int c = 0; c < channels; c+=4) {
			Engine& engine = engines[c / 4];
			dsp::SchmittTrigger::Event syncTrigger[4];
			//setting scan value and pitch across engine
			engine.scanValue = getParam(SCAN_PARAM).getValue();
			float_4 currentSampleTime = args.sampleTime;
			float_4 pitch;
			float_4 polySyncWindow;

			for (int i=0; i<4; i++) {
				polySyncWindow[i] = 4*pow((getInput(SYNC_INPUT).getPolyVoltage(c+i)/10), 2);
				syncTrigger[i] =  engine.syncCheck[c+i].processEvent(getInput(SYNC_INPUT).getPolyVoltage(c+i));
				pitch[i] = getParam(FREQ_PARAM).getValue() + getInput(VOCT_INPUT).getPolyVoltage(c+i);
				engine.scanValue[i] += getInput(SCAN_CV_INPUT).getPolyVoltage(c+i)/10.f;
				engine.fMod[i] = getInput(FM_DEPTH_INPUT).isConnected()? (getInput(FM_DEPTH_INPUT).getPolyVoltage(c+i)/10) * getParam(FM_DEPTH_PARAM).getValue() * getInput(FM_INPUT).getPolyVoltage(c+i) : getParam(FM_DEPTH_PARAM).getValue() * getInput(FM_INPUT).getPolyVoltage(c+i);
			}
			engine.clampScan = clamp(engine.scanValue);
        	engine.reflectedScan = 0.5f - engine.clampScan + 0.5f;
        	engine.previousScan = engine.currentScan;
        	engine.currentScan = scanReturn(allDivisors, activeRatios, engine.scanValue);
			
			float_4 freq = dsp::FREQ_C4 * dsp::exp2_taylor5(pitch);
			freq += dsp::FREQ_C4 * engine.fMod;
			engine.freq = freq;

			engine.phase += freq * currentSampleTime;
			engine.phase -= simd::trunc(engine.phase);

			for (int i=0; i<4; i++) {
				if (engine.previousScan[i] != engine.currentScan[i]) {
					scanPulse.trigger();
				}
			}

			simd::float_4 scanFreq = (freq / engine.currentScan) * currentSampleTime;
			engine.scanFreq = scanFreq;

			simd::float_4 reflectFreq = (freq / scanReturn(allDivisors, activeRatios, engine.reflectedScan)) * currentSampleTime;
			engine.reflectFreq = reflectFreq;

			simd::float_4 lowFreq = (freq / scanReturn(allDivisors, activeRatios, 1.f)) * currentSampleTime;
			engine.lowFreq = lowFreq;

			engine.scanPhase += scanFreq;
			engine.scanPhase -= simd::trunc(engine.scanPhase);
			engine.reflectedScanPhase += reflectFreq;
			engine.reflectedScanPhase -= simd::trunc(engine.reflectedScanPhase);
			engine.lowPhase += lowFreq;
			engine.lowPhase -= simd::trunc(engine.lowPhase);

			for (int i=0; i<4; i++) {
				if (syncTrigger[i] == 1) {
					engine.phase[c+i] = 0.f;
					engine.scanPhase[c+i] = 0.f;
					engine.reflectedScanPhase[c+i] = 0.f;
					engine.lowPhase[c+i] = 0.f;
				}
			}

			getOutput(FREQ_OUTPUT).setVoltageSimd(5.f * waveReturn(waveBlend, engine.phase, polySyncWindow), c);
			getOutput(SCAN_OUTPUT).setVoltageSimd(5.f * waveReturn(waveBlend, engine.scanPhase, polySyncWindow), c);
			getOutput(REFL_OUTPUT).setVoltageSimd(5.f * waveReturn(waveBlend, engine.reflectedScanPhase, polySyncWindow), c);
			getOutput(LOW_OUTPUT).setVoltageSimd(5.f * waveReturn(waveBlend, engine.lowPhase, polySyncWindow), c);
		}
        getOutput(FREQ_OUTPUT).setChannels(channels);
        getOutput(SCAN_OUTPUT).setChannels(channels);
        getOutput(REFL_OUTPUT).setChannels(channels);
        getOutput(LOW_OUTPUT).setChannels(channels);
        bool trigOut = scanPulse.process(args.sampleTime);
        getOutput(TRIG_OUTPUT).setVoltage(trigOut? 10.f : 0.f);
	}

	json_t* dataToJson() override {
        json_t* rootJ = json_object();

        // Save the activeRatios vector to JSON
        json_t* ratiosJ = json_array();
        for (int ratio : activeRatios) {
            json_array_append_new(ratiosJ, json_integer(ratio));
        }
        json_object_set_new(rootJ, "activeRatios", ratiosJ);

        return rootJ;
    }
    void dataFromJson(json_t* rootJ) override {

        // Load the activeRatios vector from JSON
        json_t* ratiosJ = json_object_get(rootJ, "activeRatios");
        if (ratiosJ && json_is_array(ratiosJ)) {
            activeRatios.clear();
            size_t index;
            json_t* value;
            json_array_foreach(ratiosJ, index, value) {
                activeRatios.push_back((int)json_integer_value(value));
            }
        }
    }
};


struct Scanning_frequency_division_osc_polyWidget : ModuleWidget {
	Scanning_frequency_division_osc_polyWidget(Scanning_frequency_division_osc_poly* module) {
		setModule(module);
		setPanel(createPanel(
			asset::plugin(pluginInstance, "res/scanning_frequency_division_osc_poly.svg"),
			asset::plugin(pluginInstance, "res/scanning_frequency_division_osc_poly_dark_theme.svg")
		));


		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundExtraBigBlackKnob>(mm2px(Vec(44.0, 24.5)), module, Scanning_frequency_division_osc_poly::FREQ_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(17.762, 25.88)), module, Scanning_frequency_division_osc_poly::SCAN_PARAM));
		addParam(createParamCentered<BefacoTinyKnob>(mm2px(Vec(50.52, 46.3)), module, Scanning_frequency_division_osc_poly::WAVE_SHAPE_PARAM));
		addParam(createParamCentered<BefacoTinyKnob>(mm2px(Vec(36.953, 64.9)), module, Scanning_frequency_division_osc_poly::FM_DEPTH_PARAM));
		addParam(createParamCentered<BefacoTinyKnob>(mm2px(Vec(50.52, 87.12)), module, Scanning_frequency_division_osc_poly::SYNC_WINDOW_PARAM));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(50.52, 76.9)), module, Scanning_frequency_division_osc_poly::WINDOWING_ON_PARAM, Scanning_frequency_division_osc_poly::WINDOWING_ON_LIGHT));
		addParam(createParamCentered<By1Button>(mm2px(Vec(9.983, 45.907)), module, Scanning_frequency_division_osc_poly::BY_1_PARAM));
		addParam(createParamCentered<By2Button>(mm2px(Vec(22.496, 45.907)), module, Scanning_frequency_division_osc_poly::BY_2_PARAM));
		addParam(createParamCentered<By3Button>(mm2px(Vec(9.983, 66.871)), module, Scanning_frequency_division_osc_poly::BY_3_PARAM));
		addParam(createParamCentered<By4Button>(mm2px(Vec(22.496, 66.871)), module, Scanning_frequency_division_osc_poly::BY_4_PARAM));
		addParam(createParamCentered<By5Button>(mm2px(Vec(9.983, 87.437)), module, Scanning_frequency_division_osc_poly::BY_5_PARAM));
		addParam(createParamCentered<By6Button>(mm2px(Vec(22.496, 87.437)), module, Scanning_frequency_division_osc_poly::BY_6_PARAM));
		addParam(createParamCentered<By7Button>(mm2px(Vec(9.983, 108.194)), module, Scanning_frequency_division_osc_poly::BY_7_PARAM));
		addParam(createParamCentered<By8Button>(mm2px(Vec(22.496, 108.194)), module, Scanning_frequency_division_osc_poly::BY_8_PARAM));

		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.465, 32.0)), module, Scanning_frequency_division_osc_poly::SCAN_CV_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(53.85, 34.2)), module, Scanning_frequency_division_osc_poly::VOCT_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(36.953, 46.3)), module, Scanning_frequency_division_osc_poly::WAVE_SHAPE_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(9.983, 55.266)), module, Scanning_frequency_division_osc_poly::BY_1_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(22.496, 55.266)), module, Scanning_frequency_division_osc_poly::BY_2_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(50.52, 64.9)), module, Scanning_frequency_division_osc_poly::FM_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(36.953, 74.6)), module, Scanning_frequency_division_osc_poly::FM_DEPTH_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(9.983, 76.244)), module, Scanning_frequency_division_osc_poly::BY_3_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(22.496, 76.244)), module, Scanning_frequency_division_osc_poly::BY_4_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(36.953, 87.5)), module, Scanning_frequency_division_osc_poly::SYNC_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(9.983, 96.791)), module, Scanning_frequency_division_osc_poly::BY_5_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(22.496, 96.791)), module, Scanning_frequency_division_osc_poly::BY_6_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(9.983, 117.723)), module, Scanning_frequency_division_osc_poly::BY_7_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(22.496, 117.723)), module, Scanning_frequency_division_osc_poly::BY_8_INPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(36.953, 104.15)), module, Scanning_frequency_division_osc_poly::FREQ_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(50.52, 104.15)), module, Scanning_frequency_division_osc_poly::SCAN_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(36.953, 117.1)), module, Scanning_frequency_division_osc_poly::REFL_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(50.52, 117.1)), module, Scanning_frequency_division_osc_poly::LOW_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(28.1, 32.0)), module, Scanning_frequency_division_osc_poly::TRIG_OUTPUT));
	}
};


Model* modelScanning_frequency_division_osc_poly = createModel<Scanning_frequency_division_osc_poly, Scanning_frequency_division_osc_polyWidget>("scanning_frequency_division_osc_poly");