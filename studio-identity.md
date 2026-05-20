# Studio Identity — Orient Plugins

### Studio Info
Name:            Orient Plugins
Tagline:         [leave blank — to be filled]
Contact email:   [leave blank — to be filled]
Website:         [leave blank — to be filled]

### Logo Files
Master logo:     D:\Dev\PluginSkeleton\assets\logo_black_200.png
GUI logo:        D:\Dev\PluginSkeleton\assets\logo_white_80.png
Icon:            D:\Dev\PluginSkeleton\assets\logo.ico
Marketplace:     D:\Dev\PluginSkeleton\assets\logo_marketplace_512.png
Banner:          D:\Dev\PluginSkeleton\assets\banner_1280.png

GUI logo usage:
- Position:      Bottom-right corner of every plugin editor
- Size:          80x80px maximum, scale proportionally
- Margin:        8px from right edge, 8px from bottom edge
- Load via:      ImageCache::getFromFile()
- Render in:     PluginEditor::paint() after all other components

### Color Palette

Background primary:    #F8F9FB   (main panel surface)
Background secondary:  #ECEEF3   (inset areas, knob base rings)
Accent primary:        #5B7BF8   (knobs, active states, progress, CTA)
Accent light:          #E8EBF5   (knob track empty, subtle fills)
Text primary:          #1C2035   (labels, headings, values)
Text secondary:        #8C94AD   (units, hints, secondary labels)
Border:                #D8DCE8   (panel edges, separators, dividers)

### LookAndFeel Mappings (JUCE)
Map palette to these exact JUCE ColourIds in LookAndFeel overrides:

ResizableWindow::backgroundColourId         -> #F8F9FB
Slider::backgroundColourId                  -> #ECEEF3
Slider::trackColourId                       -> #5B7BF8
Slider::thumbColourId                       -> #5B7BF8
Slider::rotarySliderFillColourId            -> #5B7BF8
Slider::rotarySliderOutlineColourId         -> #E8EBF5
Label::textColourId                         -> #1C2035
Label::backgroundColourId                   -> transparent
TextButton::buttonColourId                  -> #5B7BF8
TextButton::textColourOffId                 -> #F8F9FB
ComboBox::backgroundColourId                -> #ECEEF3
ComboBox::outlineColourId                   -> #D8DCE8
ComboBox::textColourId                      -> #1C2035
GroupComponent::outlineColourId             -> #D8DCE8
GroupComponent::textColourId                -> #8C94AD

### Typography
Label font:      Default JUCE (inter-compatible sans)
Label size:      12px
Value size:      11px
Header size:     14px
Letter spacing:  0.03em on plugin name, 0.1em on studio name
Plugin name:     font-weight 500, color #1C2035
Studio name:     uppercase, font-weight 400, color #8C94AD, size 10px

### Style Keywords
clean minimal modern playful single-accent cool-grey

### GUI Layout Rules
- Plugin name: top-left of editor header
- Studio name "ORIENT PLUGINS": top-right or below plugin name,
  uppercase, #8C94AD, 10px
- Version badge: top-right, #ECEEF3 pill background, #5B7BF8 text
- Knob value readout: below each knob label, #1C2035, 10px bold
- All knobs: double-ring style —
    outer ring: #ECEEF3 background, #D8DCE8 border
    inner ring: #F8F9FB background, #5B7BF8 border (active arc)
- Progress/level bars: #ECEEF3 track, #5B7BF8 fill, 5px height, 6px radius
- Active indicator dot: 6px circle, #5B7BF8, bottom-left of editor
- Version string: bottom-right, #D8DCE8, 9px, letter-spacing 0.06em
- Logo mark: bottom-right corner, 80x80px max, 8px margins
