#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file

extern Model* modelScanning_clock_multiplier_v2;
extern Model* modelPhase_driven_multihit_envelope;
extern Model* modelScanning_frequency_division_osc_poly;
extern Model* modelMelody_transformer;

//big bunch of mostly nearly identical component structs

struct Note_Button : app::SvgSwitch {
	Note_Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/note_off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/note_prio_4.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/note_prio_3.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/note_prio_2.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/note_prio_1.svg")));
	}
};

struct RoundExtraBigBlackKnob : RoundKnob {
	RoundExtraBigBlackKnob() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/RoundExtraBigBlackKnob.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/RoundExtraBigBlackKnob_bg.svg")));
	}
};

struct By1Button : app::SvgSwitch {
	By1Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By1off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By1on.svg")));
	}
};
struct By2Button : app::SvgSwitch {
	By2Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By2off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By2on.svg")));
	}
};
struct By3Button : app::SvgSwitch {
	By3Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By3off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By3on.svg")));
	}
};
struct By4Button : app::SvgSwitch {
	By4Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By4off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By4on.svg")));
	}
};
struct By5Button : app::SvgSwitch {
	By5Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By5off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By5on.svg")));
	}
};
struct By6Button : app::SvgSwitch {
	By6Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By6off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By6on.svg")));
	}
};
struct By7Button : app::SvgSwitch {
	By7Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By7off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By7on.svg")));
	}
};
struct By8Button : app::SvgSwitch {
	By8Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By8off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/By8on.svg")));
	}
};

struct _1_1Button : app::SvgSwitch {
	_1_1Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/1.1off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/1.1on.svg")));
	}
};
struct _8_5Button : app::SvgSwitch {
	_8_5Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/8.5off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/8.5on.svg")));
	}
};
struct _2_1Button : app::SvgSwitch {
	_2_1Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/2.1off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/2.1on.svg")));
	}
};
struct _8_3Button : app::SvgSwitch {
	_8_3Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/8.3off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/8.3on.svg")));
	}
};
struct _3_1Button : app::SvgSwitch {
	_3_1Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/3.1off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/3.1on.svg")));
	}
};
struct _16_5Button : app::SvgSwitch {
	_16_5Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/16.5off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/16.5on.svg")));
	}
};
struct _4_1Button : app::SvgSwitch {
	_4_1Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/4.1off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/4.1on.svg")));
	}
};
struct _5_1Button : app::SvgSwitch {
	_5_1Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/5.1off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/5.1on.svg")));
	}
};
struct _16_3Button : app::SvgSwitch {
	_16_3Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/16.3off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/16.3on.svg")));
	}
};
struct _6_1Button : app::SvgSwitch {
	_6_1Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/6.1off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/6.1on.svg")));
	}
};
struct _32_5Button : app::SvgSwitch {
	_32_5Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/32.5off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/32.5on.svg")));
	}
};
struct _7_1Button : app::SvgSwitch {
	_7_1Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/7.1off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/7.1on.svg")));
	}
};
struct _8_1Button : app::SvgSwitch {
	_8_1Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/8.1off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/8.1on.svg")));
	}
};
struct _32_3Button : app::SvgSwitch {
	_32_3Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/32.3off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/32.3on.svg")));
	}
};
struct _16_1Button : app::SvgSwitch {
	_16_1Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/16.1off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/16.1on.svg")));
	}
};
struct _32_1Button : app::SvgSwitch {
	_32_1Button() {
		momentary = false;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/32.1off.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/ComponentLibrary/32.1on.svg")));
	}
};

