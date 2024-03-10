# ChipTune Engine


## Instrument Definitions
### Syntax
```markdown
instrument <instrument_name> <waveform> [duty_cycle:<pwm_duty_cycle>]
 [ffx:<freq_preset_effect>] [afx:<ampl_preset_effect>] [pfx:<phase_preset_effect>] [adsr:<adsr_id>] [flp:<low_pass_filter_id>] [vol:<volume>]

instrument <instrument_name> ring_mod_A:<ring_mod_instr_A> ring_mod_B:<ring_mod_instr_B>
 [adsr:<adsr_id>] [flp:<low_pass_filter_id>] [vol:<volume>]

instrument <instrument_name> conv_A:<conv_instr_A> conv_B:<conv_instr_B>
 [adsr:<adsr_id>] [flp:<low_pass_filter_id>] [vol:<volume>]

instrument <instrument_name> ((<w0>, <instrument_name0>), (<w1>, <instrument_name1>), ...)
 [adsr:<adsr_id>] [flp:<low_pass_filter_id>] [vol:<volume>]

instrument <instrument_name> &<lib_instrument_name>
 [ffx:<freq_preset_effect>] [afx:<ampl_preset_effect>] [pfx:<phase_preset_effect>]
 [adsr:<adsr_id>] [flp:<low_pass_filter_id>] [vol:<volume>]
 ```
 Where:
```markdown
<instrument_name> := [A-Za-z][A-Za-z0-9]*
<waveform> := "sine"|"square"|"triangle"|"sawtooth"|"noise"|"pwm"
<volume> := 0..1
<lib_instrument_name> := "PIANO"|"VIOLIN"|"ORGAN"|"TRUMPET"|"FLUTE"|"GUITAR"|"KICKDRUM"|"SNAREDRUM"|"HIHAT"|"ANVIL"
```

### Examples
```markdown
instrument PIANO ring_mod_A:I0 ring_mod_B:I1 adsr:0
instrument I0 square adsr:1 flp:0 afx:VIBRATO
instrument I1 pwm duty_cycle:0.3
instrument ORGAN ((0.5, PIANO), (0.2, I0)) flp:0
instrument FLUTE_VIB &FLUTE ffx:CONSTANT afx:VIBRATO_0 pfx:ZERO vol:0.15
instrument TRUMPET &TRUMPET ffx:CONSTANT afx:CONSTANT pfx:ZERO vol:0.6
instrument TRUMPET_VIB &TRUMPET ffx:CHIRP_2 afx:VIBRATO_0 pfx:ZERO vol:0.6
instrument KDRUM &KICKDRUM
instrument SDRUM &SNAREDRUM
instrument HIHAT &HIHAT
instrument STEEL_PAN conv_A:FLUTE_VIB conv_B:SDRUM adsr:1
```

## Modulation Envelopes
### Syntax
```markdown
adsr <adsr_nr> <attack_mode> <attack_ms> <decay_mode> <decay_ms> <sustain_level> <release_mode> <release_ms>
adsr <adsr_nr> &<lib_adsr_name>
```
Where:
```markdown
<adsr_nr> := [0-9]+
<attack_mode> := <adsr_mode>
<decay_mode> := <adsr_mode>
<release_mode> := <adsr_mode>
<adsr_mode> := "LIN"|"EXP"|"LOG"
<attack_ms> := [0-9]+
<decay_ms> := [0-9]+
<release_ms> := [0-9]+
```
### Examples
```markdown
adsr 0 LIN 100 EXP 300 50 LOG 500   ; Modulation envelope adsr0 with linear attack of 100 ms, exponential decay of 300 ms, sustain level of 50, and logarithmic release of 500 ms
adsr 1 EXP 50 EXP 200 80 LIN 300
adsr 2 &ORGAN_2
adsr 3 LOG 80 LOG 30 50 LOG 10
```

## Low Pass Filters
### Syntax
```markdown
filter_lp <filter_lp_nr> [type] [order] [cutoff_frq_mult] [ripple]
```

### Examples
```markdown
filter_lp 0 Butterworth 2 1.5 0.1
```

## Note format:
```markdown
<pitch> <duration_ms> <instrument>
```

Example:
```markdown
TIME_STEP_MS 50
NUM_VOICES 3


#TAB | A4 400 STEEL_PAN |
#END
VOLUME 1
LABEL lbl0
TAB | A2 50 KDRUM | C4 250 PIANO adsr:3 |
TAB -
TAB -
TAB -
TAB | A6 20 HIHAT | - | G4 250 ORGAN adsr:3 |
TAB -
TAB -
TAB -
TAB | B4 50 SDRUM |
TAB -
TAB -
TAB -
ENDING 0
TAB | A6 20 HIHAT | E#4 250 PIANO | A4 600 FLUTE_VIB |
TAB -
TAB -
TAB -
TAB | A2 50 KDRUM |
TAB -
TAB -
TAB -
TAB | A6 20 HIHAT |
TAB -
TAB -
TAB -
ENDING 1
TAB | A6 20 HIHAT | F#4 250 PIANO | Db4 600 FLUTE_VIB vol:3.5 |
TAB -
TAB -
TAB -
TAB | A2 50 KDRUM |
TAB -
TAB -
TAB -
TAB | A6 20 HIHAT |
TAB -
TAB -
TAB -
GOTO_TIMES lbl0 1
TAB | B4 50 SDRUM | G4 500 PIANO |
TAB -
TAB -
TAB -
TAB | A6 20 HIHAT | - | Gb4 250 ORGAN |
TAB -
TAB -
TAB -
TAB | A2 50 KDRUM |
TAB -
TAB -
TAB -
SEGNO
TAB | A6 20 HIHAT | D#4 360 TRUMPET |
TAB -
TAB -
TAB -
TAB | B4 50 SDRUM | - | C4 200 TRUMPET |
TAB -
TAB -
TAB -
TAB | A6 20 HIHAT | - | A3 200 TRUMPET |
TAB -
TAB -
TAB | - | - | - |
TAB | A2 50 KDRUM | - | C4 200 TRUMPET |
TAB -
TAB -
TAB -
TAB | A6 20 HIHAT | C4 200 TRUMPET |
TAB -
TAB -
TAB -
TAB | B4 50 SDRUM | Eb4 650 TRUMPET_VIB |
TAB -
TAB -
TAB -
FINE
TO_CODA
TAB | A6 20 HIHAT |
TAB -
TAB -
TAB -
TAB | A2 50 KDRUM |
TAB -
TAB -
TAB -
TIME_STEP_MS 70
TAB | A6 20 HIHAT |
TAB -
TAB -
TAB -
TAB | B4 50 SDRUM |
TAB -
TAB -
TAB -
TIME_STEP_MS 80
TAB | A6 20 HIHAT |
TAB -
TAB -
TAB -
TAB | A2 50 KDRUM |
TAB -
TAB -
TAB -
TIME_STEP_MS 90
TAB | A6 20 HIHAT |
TAB -
TAB -
TAB -
TAB | B4 50 SDRUM |
TAB -
TAB -
TAB -
TIME_STEP_MS 100
TAB | A6 20 HIHAT |
TAB -
TAB -
TAB -
TAB | A2 50 KDRUM |
TAB -
TAB -
TAB -
#DA_CAPO_AL_CODA
#DAL_SEGNO_AL_CODA
VOLUME 0.8
TAB | A6 20 HIHAT | F#4 120 STEEL_PAN adsr:2 |
TAB -
TAB -
TAB -
VOLUME 0.6
TAB | B4 50 SDRUM |
TAB -
TAB -
TAB -
VOLUME 0.4
TAB | A6 20 HIHAT | G5 350 STEEL_PAN adsr:3 |
TAB -
TAB -
TAB -
CODA
TIME_STEP_MS 150
VOLUME 0.2
TAB | A2 50 KDRUM |
TAB -
TAB -
TAB -
TAB | A6 20 HIHAT |
TAB -
TAB -
TAB -
TAB | B4 50 SDRUM |
TAB -
TAB -
TAB -
TAB | A6 20 HIHAT | F4 350 STEEL_PAN |
TAB -
TAB -
TAB -
DA_CAPO_AL_FINE
#DAL_SEGNO_AL_FINE
```