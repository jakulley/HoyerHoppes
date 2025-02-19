#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelScanning_clock_multiplier_v2);
	p->addModel(modelPhase_driven_multihit_envelope);
	p->addModel(modelScanning_frequency_division_osc_poly);
	p->addModel(modelMelody_transformer);
	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
