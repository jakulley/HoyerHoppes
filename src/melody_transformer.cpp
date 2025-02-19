#include "plugin.hpp"

struct Melody_transformer : Module {
	enum ParamId {
		C_PARAM,
		C_SHARP_PARAM,
		D_PARAM,
		D_SHARP_PARAM,
		E_PARAM,
		F_PARAM,
		F_SHARP_PARAM,
		G_PARAM,
		G_SHARP_PARAM,
		A_PARAM,
		A_SHARP_PARAM,
		B_PARAM,
		GAIN_PARAM,
		OFFSET_PARAM,
		S_AND_H_PARAM,
		LOOP_LENGTH_PARAM,
		LOOP_ON_PARAM,
		OVERWRITE_PARAM,
		FLOOR_MODE_PARAM,
		CEILING_MODE_PARAM,
		FLOOR_LEVEL_PARAM,
		CEILING_LEVEL_PARAM,
		TRANSPOSE_PARAM,
		PRIO_SHIFT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INPUT_INPUT,
		GAIN_INPUT,
		OFFSET_INPUT,
		S_AND_H_INPUT,
		CLOCK_INPUT,
		OVERWRITE_INPUT,
		FLOOR_MODE_INPUT,
		CEILING_MODE_INPUT,
		FLOOR_LEVEL_INPUT,
		CEILING_LEVEL_INPUT,
		TRANSPOSE_INPUT,
		PRIO_SHIFT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		RAW_OUTPUT,
		PRIO_1_OUTPUT,
		PRIO_2_OUTPUT,
		PRIO_3_OUTPUT,
		PRIO_4_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		S_AND_H_LIGHT,
		LOOP_LIGHT,
		OVERWRITE_LIGHT,
		LIGHTS_LEN
	};

	Melody_transformer() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(GAIN_PARAM, -1.f, 1.f, 1.f, "");
		configParam(OFFSET_PARAM, -5.f, 5.f, 0.f, "");
		configSwitch(S_AND_H_PARAM, 0.f, 1.f, 0.f, "turns sample and hold on and off");
		configSwitch(LOOP_LENGTH_PARAM, 1.f, 16.f, 16.f, "loop length", {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"});
		configSwitch(LOOP_ON_PARAM, 0.f, 1.f, 0.f, "when off, input writes to shift register");
		configButton(OVERWRITE_PARAM, "while held, replace contents of loop with contents of input");
		configSwitch(FLOOR_MODE_PARAM, 0.f, 4.f, 4.f, "");
		configSwitch(CEILING_MODE_PARAM, 0.f, 4.f, 4.f, "");
		configParam(FLOOR_LEVEL_PARAM, -10.f, 10.f, -10.f, "");
		configParam(CEILING_LEVEL_PARAM, -10.f, 10.f, 10.f, "");
		configSwitch(TRANSPOSE_PARAM, -12.f, 12.f, 0.f, "", {"-12", "-11", "-10", "-9", "-8", "-7", "-6", "-5", "-4", "-3", "-2", "-1", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"});
		configSwitch(C_SHARP_PARAM, 0.f, 4.f, 0.f, "");
		configSwitch(D_SHARP_PARAM, 0.f, 4.f, 0.f, "");
		configSwitch(F_SHARP_PARAM, 0.f, 4.f, 0.f, "");
		configSwitch(G_SHARP_PARAM, 0.f, 4.f, 0.f, "");
		configSwitch(A_SHARP_PARAM, 0.f, 4.f, 0.f, "");
		configSwitch(C_PARAM, 0.f, 4.f, 4.f, "");
		configSwitch(D_PARAM, 0.f, 4.f, 2.f, "");
		configSwitch(E_PARAM, 0.f, 4.f, 3.f, "");
		configSwitch(F_PARAM, 0.f, 4.f, 1.f, "");
		configSwitch(G_PARAM, 0.f, 4.f, 3.f, "");
		configSwitch(A_PARAM, 0.f, 4.f, 2.f, "");
		configSwitch(B_PARAM, 0.f, 4.f, 1.f, "");
		configSwitch(PRIO_SHIFT_PARAM, -12.f, 12.f, 0.f, "shift scale while staying in key", {"-12", "-11", "-10", "-9", "-8", "-7", "-6", "-5", "-4", "-3", "-2", "-1", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"});
		configInput(INPUT_INPUT, "-10 to 10v, white noise normaled in");
		configInput(GAIN_INPUT, "-10 to 10v, attenuverted by knob");
		configInput(OFFSET_INPUT, "-10 to 10v, attenuverted by knob");
		configInput(S_AND_H_INPUT, "expects triggers");
		configInput(CLOCK_INPUT, "normaled to sample and hold input");
		configInput(OVERWRITE_INPUT, "when high, take new sample on clock trigger");
		configInput(FLOOR_MODE_INPUT, "0-5v, overrides knob");
		configInput(CEILING_MODE_INPUT, "0-5v, overrides knob");
		configInput(FLOOR_LEVEL_INPUT, "-10 to 10v, overrides knob");
		configInput(CEILING_LEVEL_INPUT, "-10 to 10v, overrides knob");
		configInput(TRANSPOSE_INPUT, "accepts v/oct");
		configInput(PRIO_SHIFT_INPUT, "accepts v/oct, ignoring notes outside of active scale");
		configOutput(RAW_OUTPUT, "unquantized, but still processed and looped (if applicable)");
		configOutput(PRIO_1_OUTPUT, "all active notes of the scale");
		configOutput(PRIO_2_OUTPUT, "e.g. the root, 3rd, and 5th of the scale");
		configOutput(PRIO_3_OUTPUT, "e.g. the root and 5th of the scale");
		configOutput(PRIO_4_OUTPUT, "e.g. the root of the scale");
	}

	struct Note {
		float value = 0.f;
		dsp::SchmittTrigger prioTrig;
		float prio = 0.f;
	};
	Note notes[12];
	Note root;
	float previousPrios[12] = {};
	int transpose = 0;
	int prevTranspose = 0;
	int prioShift = 0;
	int prevPrioShift = 0;

	std::vector<Note> prio1s;
	std::vector<Note> prio2s;
	std::vector<Note> prio3s;
	std::vector<Note> prio4s;
	float prio1Out = 0.f;
	float prio2Out = 0.f;
	float prio3Out = 0.f;
	float prio4Out = 0.f;

	dsp::SchmittTrigger sampleTrig[16];
	dsp::SchmittTrigger loopTrig[16];
	dsp::SchmittTrigger overwriteTrig[16];
	float shiftReg[16][16] = {};
	int shift = 0;
	float shiftOut[16] = {};

	float input[16] = {};
	float gain = 0.f;
	float gainRaw[16] = {};
	float offset = 0.f;
	float offsetRaw[16] = {};
	float sampledInput[16] = {};
	float processedInput[16] = {};
	float raw[16] = {};
	float floor = 0.f;
	int floorMode = 0;
	float ceiling = 0.f;
	int ceilingMode = 0;

	bool updateNotes(int transpose, int prioShift) {
		bool needsUpdate = false;
		if (prioShift != prevPrioShift || transpose != prevTranspose) {
			needsUpdate = true;
			prevPrioShift = prioShift;
			prevTranspose = transpose;
		}
	
		// Step 1: Store original prio values before transposition
		float originalPrios[12];
		for (int i = 0; i < 12; i++) {
			notes[i].value = i / 12.0f;
			originalPrios[i] = getParam(ParamId(i)).getValue();
			
			if (originalPrios[i] != previousPrios[i]) {
				needsUpdate = true;
				previousPrios[i] = originalPrios[i]; 
			}
		}
	
		if (!needsUpdate) {
			return false; // No update needed
		}
	
		// Step 2: Clear vectors (full refresh to remove outdated notes)
		prio1s.clear();
		prio2s.clear();
		prio3s.clear();
		prio4s.clear();
	
		// Step 3: Apply transposition
		float transposedPrios[12] = {};
		for (int i = 0; i < 12; i++) {
			int transposedIndex = (i - transpose + 12) % 12;
			transposedPrios[i] = originalPrios[transposedIndex];
		}
	
		// Step 4: Collect active notes (prio > 0) for prioShift
		std::vector<int> activeNotes;
		for (int i = 0; i < 12; i++) {
			if (transposedPrios[i] > 0.f) {
				activeNotes.push_back(i);
			}
		}
	
		// Step 5: Apply prioShift within active notes
		if (!activeNotes.empty()) {
			std::vector<float> shiftedPrios(activeNotes.size());
	
			// Shift priorities cyclically
			for (size_t i = 0; i < activeNotes.size(); i++) {
				int shiftedIndex = (i + prioShift + activeNotes.size()) % activeNotes.size();
				shiftedPrios[shiftedIndex] = transposedPrios[activeNotes[i]];
			}
	
			// Assign shifted priorities back to transposedPrios
			for (size_t i = 0; i < activeNotes.size(); i++) {
				transposedPrios[activeNotes[i]] = shiftedPrios[i];
			}
		}
	
		// Step 6: Populate priority lists
		for (int i = 0; i < 12; i++) {
			notes[i].prio = transposedPrios[i];
	
			if (notes[i].prio > 0.f) prio1s.push_back(notes[i]);
			if (notes[i].prio > 1.f) prio2s.push_back(notes[i]);
			if (notes[i].prio > 2.f) prio3s.push_back(notes[i]);
			if (notes[i].prio > 3.f) prio4s.push_back(notes[i]);
		}
	
		// Step 7: Ensure vectors remain sorted by `value`
		auto sortByValue = [](const Note& a, const Note& b) {
			return a.value < b.value;
		};
	
		std::sort(prio1s.begin(), prio1s.end(), sortByValue);
		std::sort(prio2s.begin(), prio2s.end(), sortByValue);
		std::sort(prio3s.begin(), prio3s.end(), sortByValue);
		std::sort(prio4s.begin(), prio4s.end(), sortByValue);
	
		return true; // Changes were applied
	}
	
	
	
	float getQuantizedValue(float raw, const std::vector<Note>& prioList) {
		float output = std::trunc(raw);
		float target = raw - output;
	
		auto it = std::lower_bound(prioList.begin(), prioList.end(), target,
			[](const Note& note, float target) { 
				return note.value < target; 
			});
	
		if (it != prioList.begin()) {
			--it; // Step back to get the largest valid value ≤ target
		}
	
		return output + it->value; // Return the adjusted value
	}
	

	void process(const ProcessArgs& args) override {
		prio1s.reserve(12);
		prio2s.reserve(12);
		prio3s.reserve(12);
		prio4s.reserve(12);

		//set lights
		getLight(S_AND_H_LIGHT).setBrightness(getParam(S_AND_H_PARAM).getValue() == 1.f? 1.0 : 0.0);
		getLight(LOOP_LIGHT).setBrightness(getParam(LOOP_ON_PARAM).getValue() == 1.f? 1.0 : 0.0);
		getLight(OVERWRITE_LIGHT).setBrightness(getParam(OVERWRITE_PARAM).getValue() == 1.f? 1.0 : 0.0);

		//set note values and prio
		transpose = int(getInput(TRANSPOSE_INPUT).isConnected()? round(getInput(TRANSPOSE_INPUT).getVoltage()*12.) : getParam(TRANSPOSE_PARAM).getValue()) % 12;
		//setting prioShift to respond to v/oct is more complicated than transpose, because it should only respond to values in the active scale
		int inputNote = round(getInput(PRIO_SHIFT_INPUT).isConnected()? getInput(PRIO_SHIFT_INPUT).getVoltage() * 12. : getParam(PRIO_SHIFT_PARAM).getValue());
		inputNote = inputNote % 12;
		if (inputNote < 0) inputNote += 12;
		// Find the index of the input note in the active scale
		std::vector<int> activeScale;
		for (int i = 0; i < 12; i++) {
			if (notes[i].prio > 0) {
				activeScale.push_back(i);
			}
		}
		// Find the first (lowest index) active note in the scale to use as reference
		int referenceIndex = -1;
		if (!activeScale.empty()) {
			referenceIndex = 0; // The first note in the sorted activeScale is the "root"
		}
		// Find index of inputNote in activeScale
		int newIndex = -1;
		for (size_t i = 0; i < activeScale.size(); i++) {
			if (activeScale[i] == inputNote) {
				newIndex = i;
				break;
			}
		}
		// Ensure prioShift is updated only if inputNote is in activeScale and there's a valid reference
		if (newIndex != -1 && referenceIndex != -1) {
			prioShift = newIndex - referenceIndex;
		}
		for (int i=0; i<12; i++) {
			if (notes[i].prio < 3.) {
				root = notes[i];
				break;
			}
		}
		updateNotes(transpose, prioShift);

		//set channels based on input
		int channels = std::max(1, getInput(INPUT_INPUT).getChannels());

		for (int c = 0; c < channels; c++) {
			input[c] = getInput(INPUT_INPUT).isConnected()? getInput(INPUT_INPUT).getPolyVoltage(c) : std::rand() / float(RAND_MAX) * 20. - 10.;
			
			dsp::SchmittTrigger::Event shTrig = sampleTrig[c].processEvent(getInput(S_AND_H_INPUT).getPolyVoltage(c));
			if (shTrig == 1) {
				sampledInput[c] = input[c];
			}
			bool sAndHOn = getParam(S_AND_H_PARAM).getValue() == 1.;
			processedInput[c] = sAndHOn? sampledInput[c] : input[c];

			//loop block
			bool loopClock = getInput(CLOCK_INPUT).isConnected();
			bool loopOn = getParam(LOOP_ON_PARAM).getValue() == 1.;
			shift = clamp(shift, 0, 15);
			dsp::SchmittTrigger::Event lTrig = loopTrig[c].processEvent(loopClock ? getInput(CLOCK_INPUT).getPolyVoltage(c) : getInput(S_AND_H_INPUT).getPolyVoltage(c));
			float overwrite = (getInput(OVERWRITE_INPUT).isConnected()? getInput(OVERWRITE_INPUT).getPolyVoltage(c) : getParam(OVERWRITE_PARAM).getValue());
			if (lTrig == 1) {
				if (!loopOn || overwrite >= 1.) {
					shiftReg[shift][c] = processedInput[c];
				} else {
					shiftOut[c] = shiftReg[shift][c];
				}
				shift++;  // Shift increments only once per clock event
			}
			int loopLength = std::floor(getParam(LOOP_LENGTH_PARAM).getValue() + 0.5);
			if (shift >= loopLength) {
				shift = 0;
			}
			raw[c] = loopOn? shiftOut[c] : processedInput[c];
			gain = (getInput(GAIN_INPUT).isConnected()? getInput(GAIN_INPUT).getVoltage() / 10 * getParam(GAIN_PARAM).getValue() : getParam(GAIN_PARAM).getValue());
			gainRaw[c] = raw[c] * gain;
			offset = (getInput(OFFSET_INPUT).isConnected()? getInput(OFFSET_INPUT).getVoltage() : getParam(OFFSET_PARAM).getValue()); 
			offsetRaw[c] = gainRaw[c] + offset;

			//floor and ceiling blocks
			floor = getInput(FLOOR_LEVEL_INPUT).isConnected() ? getInput(FLOOR_LEVEL_INPUT).getVoltage() : getParam(FLOOR_LEVEL_PARAM).getValue();
			floor *= gain;
			floor += offset;
			floorMode = getInput(FLOOR_MODE_INPUT).isConnected() ? clamp(int(getInput(FLOOR_MODE_INPUT).getVoltage()), 0, 5) : int(getParam(FLOOR_MODE_PARAM).getValue());
			ceiling = getInput(CEILING_LEVEL_INPUT).isConnected() ? getInput(CEILING_LEVEL_INPUT).getVoltage() : getParam(CEILING_LEVEL_PARAM).getValue();
			ceiling *= gain;
			ceiling += offset;
			ceilingMode = getInput(CEILING_MODE_INPUT).isConnected() ? clamp(int(getInput(CEILING_MODE_INPUT).getVoltage()), 0, 5) : int(getParam(CEILING_MODE_PARAM).getValue());
			if (offsetRaw[c] < floor) {
				switch (floorMode) {
					case 0 :
					offsetRaw[c] = ceiling;
						break;
					case 1 :
						while (offsetRaw[c] < floor) {
							offsetRaw[c]++;
						}
						break;
					case 2 :
						offsetRaw[c] += 2*(floor - offsetRaw[c]);
						break;
					case 3 :
						offsetRaw[c] = floor;
						break;
					case 4 :
						break;
				}

			}
			if (offsetRaw[c] > ceiling) {
				switch (ceilingMode) {
					case 0 :
						offsetRaw[c] = trunc(floor) + root.value;
						break;
					case 1 :
						while (offsetRaw[c] > ceiling) {
							offsetRaw[c]--;
						}
						break;
					case 2 :
						offsetRaw[c] -= 2*(offsetRaw[c] - ceiling);
						break;
					case 3 :
						offsetRaw[c] = ceiling;
						break;
					case 4 :
						break;
				}
			}

			//quantize block
			prio1Out = getQuantizedValue(offsetRaw[c], prio1s);
			prio2Out = getQuantizedValue(offsetRaw[c], prio2s);
			prio3Out = getQuantizedValue(offsetRaw[c], prio3s);
			prio4Out = getQuantizedValue(offsetRaw[c], prio4s);


			getOutput(PRIO_1_OUTPUT).setVoltage(prio1Out, c);
			getOutput(PRIO_2_OUTPUT).setVoltage(prio2Out, c);
			getOutput(PRIO_3_OUTPUT).setVoltage(prio3Out, c);
			getOutput(PRIO_4_OUTPUT).setVoltage(prio4Out, c);
			getOutput(RAW_OUTPUT).setVoltage(offsetRaw[c], c);
		}
		getOutput(PRIO_1_OUTPUT).setChannels(channels);
		getOutput(PRIO_2_OUTPUT).setChannels(channels);
		getOutput(PRIO_3_OUTPUT).setChannels(channels);
		getOutput(PRIO_4_OUTPUT).setChannels(channels);
		getOutput(RAW_OUTPUT).setChannels(channels);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
	
		// Save shiftReg as a nested JSON array
		json_t* shiftRegJ = json_array();
		for (int i = 0; i < 16; i++) {
			json_t* rowJ = json_array();
			for (int j = 0; j < 16; j++) {
				json_array_append_new(rowJ, json_real(shiftReg[i][j]));
			}
			json_array_append_new(shiftRegJ, rowJ);
		}
		json_object_set_new(rootJ, "shiftReg", shiftRegJ);
	
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* shiftRegJ = json_object_get(rootJ, "shiftReg");
		if (shiftRegJ && json_is_array(shiftRegJ)) {
			size_t rowCount = json_array_size(shiftRegJ);
			
			for (size_t i = 0; i < rowCount && i < 16; i++) { // Limit to 16 rows
				json_t* rowJ = json_array_get(shiftRegJ, i);
				if (rowJ && json_is_array(rowJ)) {
					size_t colCount = json_array_size(rowJ);
					
					for (size_t j = 0; j < colCount && j < 16; j++) { // Limit to 16 columns
						json_t* value = json_array_get(rowJ, j);
						if (json_is_number(value)) {
							shiftReg[i][j] = (float)json_real_value(value);
						}
					}
				}
			}
		}
	}
	
	
};

struct FloorModeText : Widget {
	Melody_transformer *module = nullptr;

    void setModule(Melody_transformer *module) {
        this->module = module;
    }

	void draw(const DrawArgs& args) override {
		nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 0)); // Fully transparent
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFill(args.vg);
		int mode = 0;
		if (module) {
			mode = module->floorMode;
		}
		std::string fontPath = asset::system("res/fonts/Nunito-Bold.ttf");
		std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
		std::string text = "";

		if (font) {
			nvgFontFaceId(args.vg, font->handle);
			nvgFontSize(args.vg, 12.0);
			nvgFontBlur(args.vg, -1.);
			nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_BASELINE);
			switch (mode) {
				case 0:
					text = string::f("to ceiling");
					break;
				case 1:
					text = string::f("up oct");
					break;		
				case 2:
					text = string::f("rectify");
					break;
				case 3:
					text = string::f("clip");
					break;
				case 4:
					text = string::f("bypass");
					break;	
			}
			nvgFillColor(args.vg, nvgRGB(0,0,0));
			nvgText(args.vg, 0, 0, text.c_str(), NULL);
			nvgFill(args.vg);
		}
	}
};

struct CeilingModeText : Widget {
	Melody_transformer *module = nullptr;

    void setModule(Melody_transformer *module) {
        this->module = module;
    }

	void draw(const DrawArgs& args) override {
		nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 0)); // Fully transparent
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFill(args.vg);
		int mode = 0;
		if (module) {
			mode = module->ceilingMode;
		}
		std::string fontPath = asset::system("res/fonts/Nunito-Bold.ttf");
		std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
		std::string text = "";

		if (font) {
			nvgFontFaceId(args.vg, font->handle);
			nvgFontSize(args.vg, 12.0);
			nvgFontBlur(args.vg, -1.);
			nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_BASELINE);
			switch (mode) {
				case 0:
					text = string::f("to root");
					break;
				case 1:
					text = string::f("down oct");
					break;		
				case 2:
					text = string::f("rectify");
					break;
				case 3:
					text = string::f("clip");
					break;
				case 4:
					text = string::f("bypass");
					break;	
			}
			nvgFillColor(args.vg, nvgRGB(0,0,0));
			nvgText(args.vg, 0, 0, text.c_str(), NULL);
			nvgFill(args.vg);
		}
	}
};

struct NoteShiftIndicator : Widget {
	Melody_transformer::Note* note = nullptr;

	void setNote(Melody_transformer::Note* note) {
		this->note = note;
	}

	void draw(const DrawArgs& args) override {
		if (!note) {
			return;
		}

		switch (int(note->prio)) {
			case 0:
				nvgFillColor(args.vg, nvgRGBf(0., 0., 0.));
				nvgStrokeColor(args.vg, nvgRGBf(1., 1., 1.));
				break;
			case 1:
				nvgFillColor(args.vg, nvgRGBf(0.33, 0.33, 0.33));
				nvgStrokeColor(args.vg, nvgRGBf(0.66, 0.66, 0.66));

				break;
			case 2:
				nvgFillColor(args.vg, nvgRGBf(0.5, 0.5, 0.5));
				break;
			case 3:
				nvgFillColor(args.vg, nvgRGBf(0.66, 0.66, 0.66));
				nvgStrokeColor(args.vg, nvgRGBf(0.33, 0.33, 0.33));
				break;
			case 4:
				nvgFillColor(args.vg, nvgRGBf(1., 1., 1.));
				nvgStrokeColor(args.vg, nvgRGBf(0., 0., 0.));
				break;
			default:
				nvgFillColor(args.vg, nvgRGBf(0., 0., 0.));
				nvgStrokeColor(args.vg, nvgRGBf(1., 1., 1.));
				break;
		}

		nvgBeginPath(args.vg);
		nvgCircle(args.vg, 0, 0, 5.);
		nvgFill(args.vg);
		nvgStrokeWidth(args.vg, 0.3);
		nvgStroke(args.vg);

	}
};


struct Melody_transformerWidget : ModuleWidget {
	Melody_transformerWidget(Melody_transformer* module) {
		setModule(module);
		setPanel(createPanel(
			asset::plugin(pluginInstance, "res/melody_transformer.svg"),
			asset::plugin(pluginInstance, "res/melody_transformer_dark_theme.svg")
		));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(23.319, 21.371)), module, Melody_transformer::GAIN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(43.223, 21.371)), module, Melody_transformer::OFFSET_PARAM));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(16.208, 26.198)), module, Melody_transformer::S_AND_H_PARAM, Melody_transformer::S_AND_H_LIGHT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(31.905, 41.444)), module, Melody_transformer::LOOP_LENGTH_PARAM));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(7.622, 43.786)), module, Melody_transformer::LOOP_ON_PARAM, Melody_transformer::LOOP_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(53.479, 43.786)), module, Melody_transformer::OVERWRITE_PARAM, Melody_transformer::OVERWRITE_LIGHT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(17.604, 61.626)), module, Melody_transformer::FLOOR_MODE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(49.883, 61.626)), module, Melody_transformer::CEILING_MODE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(17.604, 74.31)), module, Melody_transformer::FLOOR_LEVEL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(49.883, 74.31)), module, Melody_transformer::CEILING_LEVEL_PARAM));
		addParam(createParamCentered<BefacoTinyKnob>(mm2px(Vec(8.078, 92.449)), module, Melody_transformer::TRANSPOSE_PARAM));
		addParam(createParamCentered<Note_Button>(mm2px(Vec(30.795, 96.434)), module, Melody_transformer::C_SHARP_PARAM));
		addParam(createParamCentered<Note_Button>(mm2px(Vec(35.737, 96.434)), module, Melody_transformer::D_SHARP_PARAM));
		addParam(createParamCentered<Note_Button>(mm2px(Vec(44.463, 96.434)), module, Melody_transformer::F_SHARP_PARAM));
		addParam(createParamCentered<Note_Button>(mm2px(Vec(49.439, 96.434)), module, Melody_transformer::G_SHARP_PARAM));
		addParam(createParamCentered<Note_Button>(mm2px(Vec(54.416, 96.434)), module, Melody_transformer::A_SHARP_PARAM));
		addParam(createParamCentered<Note_Button>(mm2px(Vec(28.467, 100.138)), module, Melody_transformer::C_PARAM));
		addParam(createParamCentered<Note_Button>(mm2px(Vec(33.092, 100.138)), module, Melody_transformer::D_PARAM));
		addParam(createParamCentered<Note_Button>(mm2px(Vec(37.862, 100.138)), module, Melody_transformer::E_PARAM));
		addParam(createParamCentered<Note_Button>(mm2px(Vec(42.343, 100.138)), module, Melody_transformer::F_PARAM));
		addParam(createParamCentered<Note_Button>(mm2px(Vec(46.968, 100.138)), module, Melody_transformer::G_PARAM));
		addParam(createParamCentered<Note_Button>(mm2px(Vec(51.594, 100.138)), module, Melody_transformer::A_PARAM));
		addParam(createParamCentered<Note_Button>(mm2px(Vec(56.219, 100.138)), module, Melody_transformer::B_PARAM));
		addParam(createParamCentered<BefacoTinyKnob>(mm2px(Vec(8.078, 102.986)), module, Melody_transformer::PRIO_SHIFT_PARAM));

		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.802, 16.013)), module, Melody_transformer::INPUT_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(33.029, 21.371)), module, Melody_transformer::GAIN_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(53.016, 21.371)), module, Melody_transformer::OFFSET_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.607, 25.241)), module, Melody_transformer::S_AND_H_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(18.949, 42.251)), module, Melody_transformer::CLOCK_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(46.186, 42.251)), module, Melody_transformer::OVERWRITE_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.522, 61.993)), module, Melody_transformer::FLOOR_MODE_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(39.801, 61.993)), module, Melody_transformer::CEILING_MODE_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.522, 75.402)), module, Melody_transformer::FLOOR_LEVEL_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(39.801, 75.402)), module, Melody_transformer::CEILING_LEVEL_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(17.669, 92.438)), module, Melody_transformer::TRANSPOSE_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(17.669, 102.976)), module, Melody_transformer::PRIO_SHIFT_INPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.667, 116.625)), module, Melody_transformer::RAW_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(18.26, 116.625)), module, Melody_transformer::PRIO_1_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(29.691, 116.625)), module, Melody_transformer::PRIO_2_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(40.953, 116.625)), module, Melody_transformer::PRIO_3_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(52.826, 116.625)), module, Melody_transformer::PRIO_4_OUTPUT));

		
		FloorModeText* floorModeText = createWidget<FloorModeText>(mm2px(Vec(7.604, 56.626)));
		floorModeText->setModule(module);
		addChild(floorModeText);
		CeilingModeText* ceilingModeText = createWidget<CeilingModeText>(mm2px(Vec(39.883, 56.626)));
		ceilingModeText->setModule(module);
		addChild(ceilingModeText);

		if (module) {
			for (int i = 0; i < 12; i++) {
				Vec positions[12] = {
					mm2px(Vec(28.467, 102.138)), mm2px(Vec(30.795, 94.434)), // C, C#
					mm2px(Vec(33.092, 102.138)), mm2px(Vec(35.737, 94.434)), // D, D#
					mm2px(Vec(37.862, 102.138)), mm2px(Vec(42.343, 102.138)), // E, F
					mm2px(Vec(44.463, 94.434)), mm2px(Vec(46.968, 102.138)), // F#, G
					mm2px(Vec(49.439, 94.434)), mm2px(Vec(51.594, 102.138)), // G#, A
					mm2px(Vec(54.416, 94.434)), mm2px(Vec(56.219, 102.138))  // A#, B
				};
		
				NoteShiftIndicator* indicator = createWidget<NoteShiftIndicator>(positions[i]);
				if (indicator) {
					indicator->setNote(&(module->notes[i]));
					addChild(indicator);
				}
			}
		}
		
	}
};


Model* modelMelody_transformer = createModel<Melody_transformer, Melody_transformerWidget>("melody_transformer");