Modules for VCVRack v2.0 or higher.

Included modules:

    Scanning Clock Multiplier v2

    Scanning Frequency Division Oscillator

    Phase-Driven Multihit Envelope

    Melody Transformer

![image](https://github.com/user-attachments/assets/381255ec-0672-4cfe-a7f6-9868a51115d1)
![image](https://github.com/user-attachments/assets/2b8c8c2a-c92e-44e2-85b5-1ea9f0fd964d)



**Scanning Clock Multiplier v2**

![image](https://github.com/user-attachments/assets/4bea70ba-ca9f-4e53-acf1-e3d180a48c95)

The Scanning Clock Multiplier is a clock generator or clock modulator that operates on the concept of a user-defined list of "active ratios." The buttons and their corresponding gate inputs determine whether a given ratio is active or not, and the scan parameter allows users to scan through that list via the knob and/or 0-10v CV. By selecting your active ratios thoughtfully, you can orchestrate whole rhythm sections with just one or two of these modules.

clock rate and reset section:

![image](https://github.com/user-attachments/assets/82680d2c-ac55-4f28-8543-73fabdf2c7af)

With no rate input, the rate is defined by the knob, and ranges from 1-700 bpm, defaulting to 120. The rate input can accept either clock triggers (with the direct phase button off) or 0-10 phase input (with the direct phase button on). Direct phase is useful for daisy-chaining modules, and ensures the tightest sync; if clock pulses are sent instead, the module will require two pulses before it can calculate the rate, and any tempo changes will be delayed by a pulse before they take effect.
The reset in, out, and button work exactly like every other module, resetting the phase to 0 while the gate is held.

active ratio definition section:

![image](https://github.com/user-attachments/assets/d47c433b-3b66-4cd5-bce5-35111ced31ab)

The raison of the module. Each active ratio is added to a list of ratios from which the scanner and the various other outputs select. In this example, the 1:1, 16:5, 32:5, and 32:1 ratios are active. This means the scanner will scan through 4 regions, each containing one active ratio. If the scan value is <.25, it will return 1:1; if scan <.5, it will return 16:5; and so on. The low output will return the lowest ratio (1:1), the high output returns the highest, and the middle the middle. More on the outputs in a second. The inputs below each button are gates with a 1v threshold, and have no effect if the button is already on.

The non-integer ratios are unique in that they have a tail-end phase that doesn't complete before the cycle resets; they have familiar and useful rhythmic qualities.

Note that unlike the more common clock divider modules, each ratio here *increases* the rate of the clock. For that reason it often makes sense to have a very slow master clock.

scan section:

![image](https://github.com/user-attachments/assets/5089fddd-cf85-4481-bd73-90588be471b0)

scan: 

Think of the scan knob's range as being divided into a number of regions equal to the number of active ratios (as it quite literally is in the code). As the knob (or the input 0-10v CV) passes into each new region, the scan output equals that new value. If a sawtooth lfo is fed in, then it will spend the same amount of time in each region, but of course you're not limited to sawtooth waves. Stepped random voltage, s&h outputs, triangle and sine waves, and even v/oct (given a big enough keyboard) can all yield interesting results.

skew: 

applies an exponential function to the input phase, resulting in outputs that bunch up toward the beginning or end of the phase. for a 2:1 ratio, low amounts can sound like swing. At high ratios this can create interesting enveloping or burst effects.

![image](https://github.com/user-attachments/assets/6fece706-b6b8-4c1a-8988-703592efac1b)

skewed phase vs. normal phase.

phase: 

pushes the start and end of the phase farther "back" relative to the input phase.

![image](https://github.com/user-attachments/assets/8f6a6321-460d-4abf-86d5-27b0e6a994b9)

In the first oscilloscope, two phases with no phase applied. You can see they are identical. In the second, a phase shift of .5 is applied to the second signal.
Useful in cases where syncopation is desired, as in the case of a bass groove sitting in the pocket of a kick drum, for example.
Note that phase is applied based on the active ratio in the scanner, which can result in odd behavior from the ratios present at other outputs.


output section:

![image](https://github.com/user-attachments/assets/ba622242-e702-40ac-8cba-fb6206090f4c)

The outputs on the left are triggers, the outputs on the right are phasors.

rate:

The first outputs are through-puts of the rate, with no modulation applied.

scan:

Can be thought of as the "main" output of the module, its output is determined by the list of active ratios and the value of the scan knob summed with the scan cv input.

refl. scan:

Returns the ratio at the mirror image of the knob's position. At scan = 0, it returns the value of scan = 1. At scan = .75, it returns the value of scan = .25. And so on.

low:

Returns the lowest active ratio.

high:

Returns the highest active ratio.

middle:

Returns the middlemost active ratio. If there are an even number of active ratio, it chooses the higher of the two middlemost values.

**Scanning Frequency-Division Oscillator**

A fully-featured polyphonic fm oscillator implementing a scannable list of subdivisions for unique undertone-series effects.

![image](https://github.com/user-attachments/assets/b99cf9bb-d0dc-4d31-9321-0f4558bad6d1)

scan section:

![image](https://github.com/user-attachments/assets/ae1c5640-6b65-4284-b4b9-7e6858e6e3a8)

works identically to the scan section of the scanning clock multiplier (see above for a detailed explanation), with the exception that the skew and phase parameters are absent, and instead a trigger output is present. The trigger fires everytime the scan enters a new "region" and starts returning a new subdivision. This is useful because the scanner can be used for arpeggio-like effects, and the trig out enables you to easily envelope those arpeggiated notes.

subdivision select section:

![image](https://github.com/user-attachments/assets/4c673af1-50a4-44c2-ba97-45d864734249)

Again, this is virtually identical to the scanning clock multiplier, with the exception that it has half as many selectable ratios. Furthermore, it's predicated on division rather than multiplication, and it uses only integer divisors.
Note that some divisors (3, 5, and especially 7) have an odd "out of tune" sound to our western ears, despite being perfect divisions. This is because perfect division is the province of just intonation, whereas for centuries our music has been largely composed with equal temperament. I'm not an expert, but you can google those terms to learn more about it.
Also worth noting: the subdivisions quickly get very low in pitch-- /8 is fully 4 octaves lower than the starting frequency!

The frequency knob and v/oct input work as expected. The module accepts a polyphonic input. However, the module was initially conceptualized as a monophonic oscillator whose virtue was the capacity to create multiple harmonically-related frequencies from a single v/oct source. Thus, because each voice is processed 4 times (once for each output), they come at a higher cpu price. On my computer, a Scanning Frequency-Division Oscillator uses roughly 1.5-2x more cpu than a VCV WT VCO. If you're making use of frequency divisions, that's a good trade-off, but if you just want a standard oscillator and cpu is at a premium, then there are more efficient options.

wave-shaping section:

![image](https://github.com/user-attachments/assets/c1d0f898-c7b2-41ec-bbe1-bf8ef050e595)

The module implements a classic sine-square waveshaper, passing through triangle and saw as intermediate shapes.

FM:

true through-zero FM, meaning that it maintains a strong fundamental even at extreme values. And the values can get extreme:

![image](https://github.com/user-attachments/assets/3f6d4bd1-3934-41c9-b0b9-a936af3c3aad)

The bottom wave is a VCV WT VCO at max fm depth; the top wave is a Scanning Frequency-Division Oscillator at max depth.
FM is also polyphonic, but it will distribute a monophonic fm signal across all channels of a polyphonic v/oct signal.

sync:

when the window light is off, behaves as ordinary hard sync, useful for saw and square waves, but a bit buzzy for sine and triangle waves. With the window light on, it becomes **windowed sync**, which has a lovely sound when it pairs sine waves. I was inspired to implement this feature by [Jakob Ciupinski's video](https://www.youtube.com/watch?v=Lh-yalj7-zM&ab_channel=JakubCiupinski) on smooth sync. I highly recommend his videos to every VCVRack user.

![image](https://github.com/user-attachments/assets/7fdbfe13-ad59-4410-b37c-0bdfb7872e39)

Note the harsh edges in the synced wave in the first example, vs the smooth transitions in the second example.

The window knob progressively inverts the value of the wave that's "in the window." It's usually a pretty subtle timbral effect, although it can be very pronounced with some waves.

![image](https://github.com/user-attachments/assets/902d1dd2-0de3-41a9-8265-ef5699a33ebd)

various window settings on the same synced wave.

Sync strongly affects the perceived fundamental of the synced oscillator. As such, when using sync, the subdivisions will often sound more like timbral variations, rather than genuinely different notes.

output section:

![image](https://github.com/user-attachments/assets/21955d44-4cfc-43b7-a34b-2e1d40804320)

freq:

The main frequency, unaffected by scan.

scan:

The main frequency divided by the active subdivision at scan.

refl.:

Like the scanning clock multiplier, this returns the subdivision opposite the current scan value.

low:

The lowest active subdivision. Useful for basses.





**Phase-Driven Multihit Envelope**

![{D60FBC3D-B95D-4A34-9EB5-A035D62D94B7}](https://github.com/user-attachments/assets/c59d697c-cf23-40d4-ab76-3df02630fc9e)

The Phase-Driven Multihit Envelope takes either a 0-10v phase signal (basically, a sawtooth wave) or a clock signal, and outputs a series of 1-16 envelopes. It is not an envelope generator in the traditional sense of modulating a gate from a keyboard; instead, it is tempo-driven. It might be useful to think of it as a function generator.

The graphic display: 

![Rack_43ze0qgopO](https://github.com/user-attachments/assets/cf9dc82a-8e61-4121-a590-63e56bd2c172)

With a little bit of CV, things can quickly get interesting!

The gray triangle (or blob, if the spread value is low) represents the current state of the phase after applying skew and spread. The phase output will look the same through an oscilloscope. The gray vertical line, which is only active when an input is present at rate, indicates the current position of the phase. The white line depicts the current state of the envelope--here there are four hits on both sides of the skew, affected by moderate spread and phase->envelope. It isn't perfect--see below at attack and phase->envelope--but it gives a good approximation.

rate: 

input signal. With the rate button turned on (glowing white), the output values will be the envelope calculation times the current voltage of the input--a sawtooth wave (rising from 0-10v) will scan linearly through the envelopes; a triangle wave will scan linearly forwards, then linearly backwards; and so on. With the rate button turned off, the module will calculate the length of a phase based on the period of time between two clock triggers, and then scan linearly through that period(behaving as though the clock were a sawtooth). The primary benefit of using a phase wave rather than a clock is that the module will always be synced to the input--you can change the frequency of the input on the fly and the module adapts in real time. With a standard clock signal, it will always be one cycle behind any rate changes, as it has to recalculate the phase. It also takes two clock pulses to initialize, instead of beginning instantly. But it's fine if you expect the tempo to stay steady and find the word "phase-driven" intimidating.

vca in: 

the Phase-Driven Multihit Envelope has an internal VCA normalled to the envelope, so you can modulate an incoming signal (like a sound generator) without needing an external VCA. Not important in the world of VCVRack, where modules are free, but it's easy to implement, so why not include it.

attack: 

behaves similarly to attack on a normal envelope, except that it's calculated as a percentage of the phase divided by the number of hits, rather than as a length of time. The exponential knob to the left of the slider works as expected, although its effects are not rendered on the graphics widget. To visualize the effect it's having on the envelope, send the envelope output to an oscilloscope.

![{28A2E803-5051-4EE2-B026-8A596657F16D}](https://github.com/user-attachments/assets/7a3dab78-de2b-430d-a8ed-d57de5c3300a)

the exponential knobs on attack and decay are not reflected in the graphics widget, but they work as expected


sustain: 

the sustain level is always 100% (relative to phase->env--see below); unlike most envelope modules, this slider determines the DURATION of the envelope, again calculated as a percentage of the phase divided by the number of hits.

decay: 

like attack.

phase->env: 

reduces the amplitude of the envelope by an amount proportionate to the phase value. Easier to see than to describe:

![{44710D33-F918-40B1-BFE2-662FE0B00BD6}](https://github.com/user-attachments/assets/c9a313b8-a2a6-43e2-9dfd-d96e0afd5e74)

The top scope shows the envelope output with phase->env at 100%, the bottom at 0%. You can also compare the output of the first module to the graphics widget to see how the widget misrepresents the actual output signal.



hits: 

this controls how many times the envelope triggers per phase cycle. By default it ranges from 1-16, but there are several other options selectable from the context menu, which may be useful in different rhythmic contexts, particularly when CV is applied. It works by dividing the y-value of the phase into equal portions; as the phase rises through those portions, an envelope is output.

![{A72A9254-6F29-4841-98E9-1AADA895D991}](https://github.com/user-attachments/assets/d2d2619e-0630-486d-b0b4-ce411460ed5e)

context menu providing several options for hit scaling


skew: 

![{7AF39E20-07EC-438C-ABAE-345B2CD6233C}](https://github.com/user-attachments/assets/5706ebcb-0a1b-4ab1-83d4-42b1ad04b850)

skew at 1., skew at .66, and skew at 0.

An unusual consequence of the mirroring effect of skew is that during the descending phase, the attack and decay values are swapped; attack becomes decay, and vice versa. This can create interesting and complex effects, but it may not always be desirable, e.g. in percussive contexts. To compensate for this behavior, there is a trigger output available--you can use the module to calculate complex timings, but then send triggers to a concrete gate- or trigger-based envelope.

spread: 

![{E2B3F275-A4BB-4D95-B663-07EC62D61438}](https://github.com/user-attachments/assets/c643cce0-836f-4d7d-8b53-5f2c36918a1a)

no spread, logarithmic spread, exponential spread

spread applies a variable exponential or logarithmic function to the phase--the effect this has on the envelope is to cause it to linger longer on some envelopes, and move more quickly through others. Extreme values may cause some envelopes to disappear entirely. Again, this effect is easier to understand visually, and is accurately depicted on the graphics widget. And again, because this effect distorts the envelopes--although the effect can be quite organic--you can compensate by using the trigger output to trigger an external envelope generator.

env: 

the main envelope output. 0-10v.

trig: 

outputs triggers at the beginning of each attack phase. Useful if you want to use the unique timing effects of the module, but without the attendant envelope distortions.

vca: 

outputs the value of the vca input, multiplied by the envelope.

phase: 

outputs the phase after applying skew and spread--the shape indicated by a gray triangle on the widget.


**Melody Transformer**

![image](https://github.com/user-attachments/assets/d0839803-a97f-4751-a5a2-d0d31d057ac4)

The Melody Transformer is an advanced polyphonic quantizer with numerous pre-quantization signal-processing utilities, including a 16-step polyphonic shift register. Its unique feature is the **Note Priority System**, which allows users to quickly define chords or subscales of the active scale.

Process section:

![image](https://github.com/user-attachments/assets/56f73721-5a0f-47af-9938-16899e65b65a)


Input:

The input is polyphonic; if you're using the module polyphonically, you will likely want to rescale your inputs individually before merging them and sending them to the input. With no input signal present, a single channel of **white noise is normalled to the input**, meaning the module can create its own sequences with just a clock input at s&h.

s&h:

a polyphonic trigger input for clocks or other trigger sources, the sample and hold will sample the value(s) at the input(s) once per trigger. Only active when the button is on.

gain:

an attenuverter parameter, allows you to limit the range of your input signal and/or invert it. Applies equally to all input channels.

offset:

additively shifts the range of the input after gain is applied, enabling higher- or lower-pitched sequences. Applies equally to all input channels.

If you are using the module polyphonically to orchestrate voices across multiple frequency ranges, it will likely be the case that you want to leave the gain and offset knobs alone, and instead preprocess each input signal using for example VCV RESC or the Befaco Dual Atenuverter (sic).

Loop section:

![image](https://github.com/user-attachments/assets/12514332-2a60-4823-b494-6d986c408c15)

On:

when loop is off (gray button), it writes each input channel to a separate 16-step shift register at a rate set by the clock input (or s&h if no clock input is present). When loop is on (white button), it stops writing to the shift registers, and instead starts reading them to the outputs, again at a rate set by either clock or s&h.
Gain and Offset are applied post shift register, so locked loops can still be rescaled.
The contents of the shift registers are saved on shutdown, and assuming the loop button is on, will still be there when the patch is reloaded. There is currently no facility for saving preset loops.

Clock:

Accepts clock triggers just like s&h, but is normalled to s&h, so in most use cases it can be left empty.

Length:

Determines how many steps of the shift registers to read before resetting.
A shorter loop length does not erase the content of the shift register, which is always 16 values. So you can play around with shorter loop lengths and then return to the full-length loop at any time.

Overwrite:

If the button is held (or the input is high) when loop is on and a clock trigger is fired, this will replace the contents of the shift register with the input value. Similar to the write function of a Turing Machine.

Floor and Ceiling sections:

![image](https://github.com/user-attachments/assets/d951414a-ab85-411a-be2d-6a0d66103ed1)

Floor and Ceiling work in tandem with the gain and offset parameters to limit the range of values passed by the outputs. Unlike the simple rescaling functions, Floor and Ceiling work by detecting when a value is lower or higher than their respective thresholds,
and recalculating that value based on their mode.

Mode:

There are 5 modes for both floor and ceiling.

to ceiling (floor only):

if a note is lower than the floor threshold, it gets sent to the ceiling value.

to root (ceiling only):

sends the value to the root note belonging to the floor's octave. The root note is defined as the lowest note with priority 4 (white). A very useful mode for adding more root notes to a bass ostinato.

up oct (and down oct):

if a value would exceed the relevant threshold, 1v (equivalent to an octave in v/oct) is added or subtracted from it until it no longer exceeds the threshold. In other words, the values will retain their note value, but stay within the range permitted by floor and ceiling.

rectify:

if a value would exceed the relevant threshold, it is instead raised (or reduced) by the amount by which it exceeded it.

![image](https://github.com/user-attachments/assets/f31ab83f-9dc4-4389-b046-55d169c99355)

Below, a 0-10v saw wave. Above, the effects of ceiling set to rectify and a level of 5v.

clip:

if a value would exceed the relevant threshold, it instead becomes equal to that threshold. Like to floor for ceiling, clip for floor can be useful for setting pedal points and increasing the frequency with which the input value returns to it.

![image](https://github.com/user-attachments/assets/09ca4a28-6837-47ae-85fd-9ed1c66677f1)

Below, a 0-10 saw wave. Above, the effects of ceiling set to clip anything past 5v.

bypass: 

do nothing if a value exceeds the threshold. This may seem like a useless mode--why not simply leave the levels at minimum and maximum, respectively? But, for example, bypass allows you to set a floor value which will be made use of by the ceiling's to floor function, while still allowing notes that are *less* than floor to pass by unchanged.

Like gain and offset, **floor and ceiling are applied uniformly across channels**, and have fewer use cases in complex polyphonic orchestrations.

Quantize section:

![image](https://github.com/user-attachments/assets/94bb5137-1d98-4696-8920-d8398282feb7)

Like other quantizers with keyboard-style note selects, you click notes to add or remove them from the active scale. Unlike other quantizers, you have access to **a primary scale, plus up to three subscales, each of which are sub-subscales of each other**. Combined with the priority-shift function, this enables easy chord changes without ever falling out of key.

Transpose:

semitone transposition, like you get with many other quantizers. Accepts v/oct, and wraps back around past 1v, so it will work as expected with the entire range of your keyboard. If you are modulating transpose or prioShift with non-v/oct sources (e.g. an lfo) it is recommended that you scale them to fit a -1 to 1v range, unless you want it to wrap around multiple times in a single lfo cycle.

Prio-shift:

This awkward name denotes shifting the "priority value" of each active note in the primary scale, without adding any notes that weren't already active. Like transpose, it accepts v/oct and wraps around, however it only accepts v/oct that falls within the active scale (a D# won't do anything if the scale is set to C Major, for example). Furthermore, the knob only transposes to valid values. In a C Major scale, setting the knob to +1 doesn't do anything, because D# isn't in the scale. +2 shifts you into the second mode, however. As a final note, it will still respond to unquantized voltages (e.g. from VCV SEQ-3) *as long as they round to a valid scale note*.

![image](https://github.com/user-attachments/assets/c79f4daa-0642-4f57-ad2e-793e4c9be016)

The dots show the current priority values of the notes after applying transposition and/or priority shifting.

This image clearly shows the difference between transposition and prio-shift. Both shift the root note to D, but transposition shifts all note priorities equally. In other words, we've gone from C Major to D Major (note that c# and f# are now active). Prio-shift, on the other hand, keeps all notes in the original scale, and we go from C Major to D Dorian, which is the second mode of the Major Scale.

**The effects of prio-shift won't be heard on the prio1 output**, because prio1 outputs any active note without biasing toward any priority value. But the higher priority outputs will be affected.

**By combining transpose and prio-shift, we can achieve any mode in any key without touching the note select interface.** For example, let's say I want f# minor.

Starting from C Major:

![image](https://github.com/user-attachments/assets/540e9f34-0c12-4e17-a349-9d913fa77375)

We prio-shift up to the 6th Mode, which is Aeolian, or more commonly known as the minor key. Now we are in A Minor:

![image](https://github.com/user-attachments/assets/ff0663da-490a-4d0d-9090-94b3a5fe6526)

Now we simply transpose down to f#:

![image](https://github.com/user-attachments/assets/7ddcd67a-4f27-458f-b5e0-5c3b921683b5)

And we're done! Leaving transpose alone, we can now apply v/oct to prio-shift and achieve different chords while staying in f# Minor as the overarching key.

The default settings give us the root note in the prio4 position, root+5 at prio3, root+3+5 (relative triad) at prio2, and the full scale at prio1. But there is no rule saying you have to stay in that configuration. For example:

![image](https://github.com/user-attachments/assets/2a772b36-a015-43bc-b660-a75938e51a0c)

This gives us the root at prio4, the relative triad at prio3, **the relative pentatonic at prio2**, and all notes at prio1. In fact, this may be a more useful default configuration, so I might change the default setting.

Output section:

![image](https://github.com/user-attachments/assets/dc215733-9168-4000-aa21-10b94fa83b61)

We've already discussed the outputs at some length, but to summarize:

Raw:

Raw isn't actually raw, I confess. It is the gained, offset, looped, floored, and ceilinged output. It only skips the quantizer.

Prio1:

All notes, from dark grey to white. Black notes are excluded from all outputs.

Prio2:

middle grey, light grey, and white notes.

prio3:

light grey and white notes.

prio4:

only white notes.










