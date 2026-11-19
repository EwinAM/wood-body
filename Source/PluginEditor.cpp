#include "PluginEditor.h"

namespace col
{
    const juce::Colour panelTop (0xfff2f0eb), panelBot (0xffdedad3), ink (0xff2b2a27), muted (0xff7d786e);
    const juce::Colour lcdTop (0xff161d22), lcdBot (0xff0f1418), glow (0xff9fe6cf), orange (0xffe9973a);
    const juce::Colour woodA (0xff4a2c17), woodB (0xff7a4a28), woodC (0xff5a351c), walnut (0xff9a5a2c);
    const juce::Colour swatch[3] = { juce::Colour (0xffd6b47e), juce::Colour (0xff8a4a2a), juce::Colour (0xff4a2a20) };
}

static juce::Font uiFont (float size, bool bold = true)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultSansSerifFontName(), size,
                                          bold ? juce::Font::bold : juce::Font::plain));
}
static juce::Font monoFont (float size)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain));
}

// ---------------------------------------------------------------- LookAndFeel
WoodLookAndFeel::WoodLookAndFeel()
{
    setColour (juce::TooltipWindow::textColourId, juce::Colour (0xffe7f5ef));
}

void WoodLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                        float start, float end, juce::Slider& s)
{
    const bool big = s.getName() == "big";
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const float size = juce::jmin (b.getWidth(), b.getHeight());
    const auto c = b.getCentre();
    const float R = size * 0.5f;
    const float arcR = big ? R * 0.70f : R * 0.86f;
    const float thick = big ? 6.0f : 3.5f;
    const float ang = start + pos * (end - start);

    // tick marks + stage labels (big knob)
    if (big)
    {
        g.setColour (juce::Colour (0xffa39d92));
        for (int i = 0; i <= 20; ++i)
        {
            const float a = start + (end - start) * (float) i / 20.0f;
            const float len = (i % 5 == 0) ? 7.0f : 3.5f;
            juce::Point<float> p0 = c.getPointOnCircumference (R * 0.79f, a), p1 = c.getPointOnCircumference (R * 0.79f + len, a);
            g.drawLine ({ p0, p1 }, i % 5 == 0 ? 1.6f : 1.0f);
        }
        const char* stages[] = { "0", "SUBTLE", "WARM", "THICK", "HUGE" };
        g.setFont (uiFont (8.5f));
        g.setColour (col::muted);
        for (int i = 0; i < 5; ++i)
        {
            const float a = start + (end - start) * (float) i / 4.0f;
            auto p = c.getPointOnCircumference (R * 0.95f, a);
            g.drawText (stages[i], juce::Rectangle<float> (54, 12).withCentre (p), juce::Justification::centred);
        }
    }

    // track + value arc
    juce::Path track; track.addCentredArc (c.x, c.y, arcR, arcR, 0, start, end, true);
    g.setColour (juce::Colour (0xffcfcac1));
    g.strokePath (track, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    if (pos > 0.002f)
    {
        juce::Path val; val.addCentredArc (c.x, c.y, arcR, arcR, 0, start, ang, true);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffc99a5b), c.x - arcR, c.y, col::orange, c.x + arcR, c.y, false));
        g.strokePath (val, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3D body
    const float kr = big ? arcR * 0.80f : arcR * 0.72f;
    g.setColour (juce::Colours::black.withAlpha (0.28f));
    g.fillEllipse (juce::Rectangle<float> (kr * 2.15f, kr * 2.15f).withCentre (c.translated (0, kr * 0.12f)));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a4844), c.x, c.y - kr, juce::Colour (0xff121110), c.x, c.y + kr, false));
    g.fillEllipse (juce::Rectangle<float> (kr * 2, kr * 2).withCentre (c.translated (0, kr * 0.05f)));
    const float topR = kr * 0.88f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff5c5a55), c.x - topR * 0.4f, c.y - topR * 0.6f,
                                             juce::Colour (0xff1a1917), c.x + topR, c.y + topR, true));
    g.fillEllipse (juce::Rectangle<float> (topR * 2, topR * 2).withCentre (c.translated (0, -kr * 0.04f)));

    if (big)
    {
        // knurled edge
        g.setColour (juce::Colours::white.withAlpha (0.07f));
        for (int i = 0; i < 48; ++i)
        {
            const float a = juce::MathConstants<float>::twoPi * (float) i / 48.0f + ang;
            g.drawLine ({ c.getPointOnCircumference (topR * 0.94f, a), c.getPointOnCircumference (topR, a) }, 2.0f);
        }
        // walnut cap with growth rings
        const float wr = topR * 0.68f;
        auto cap = juce::Rectangle<float> (wr * 2, wr * 2).withCentre (c);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff9a6236), c.x - wr * 0.3f, c.y - wr * 0.3f,
                                                 juce::Colour (0xff5e3820), c.x + wr, c.y + wr, true));
        g.fillEllipse (cap);
        g.setColour (juce::Colour (0xff281408).withAlpha (0.35f));
        for (int i = 1; i <= 4; ++i)
        {
            const float k = (float) i / 4.6f;
            juce::Path ring;
            ring.addEllipse (juce::Rectangle<float> (wr * 2 * k * 0.92f, wr * 2 * k).withCentre (c.translated (-wr * 0.08f * (1 - k), wr * 0.05f)));
            ring.applyTransform (juce::AffineTransform::rotation (ang, c.x, c.y));
            g.strokePath (ring, juce::PathStrokeType (1.0f));
        }
        g.setColour (juce::Colour (0xff2b180b));
        g.drawEllipse (cap, 1.4f);
    }
    // highlight
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.fillEllipse (juce::Rectangle<float> (topR * 0.9f, topR * 0.38f).withCentre (c.translated (-topR * 0.25f, -topR * 0.55f)));
    // pointer
    g.setColour (juce::Colours::white);
    const float p0 = big ? topR * 0.72f : topR * 0.30f, p1 = big ? topR * 0.95f : topR * 0.88f;
    g.drawLine ({ c.getPointOnCircumference (p0, ang), c.getPointOnCircumference (p1, ang) }, big ? 5.0f : 2.6f);
}

void WoodLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState();
    if (b.getName() == "tips")
    {
        auto led = juce::Rectangle<float> (10, 10).withCentre ({ r.getX() + 7, r.getCentreY() });
        if (on) { g.setColour (juce::Colour (0xff5fe0b0).withAlpha (0.35f)); g.fillEllipse (led.expanded (3)); }
        g.setColour (on ? juce::Colour (0xff5fe0b0) : juce::Colour (0xff8d8a83));
        g.fillEllipse (led);
        return;
    }
    if (! on)
    {
        g.setColour (juce::Colour (0xffa9a399));
        g.fillRoundedRectangle (r.translated (0, 2), 6);
    }
    auto face = on ? r.translated (0, 2) : r;
    g.setGradientFill (on ? juce::ColourGradient (juce::Colour (0xffe6e2da), 0, face.getY(), juce::Colour (0xfff2efe9), 0, face.getBottom(), false)
                          : juce::ColourGradient (juce::Colour (0xfffdfcfa), 0, face.getY(), juce::Colour (0xffdcd8d0), 0, face.getBottom(), false));
    g.fillRoundedRectangle (face, 6);
    g.setColour (juce::Colour (0xffb9b4aa));
    g.drawRoundedRectangle (face, 6, 1);
    if (over && ! on) { g.setColour (juce::Colours::white.withAlpha (0.25f)); g.fillRoundedRectangle (face, 6); }
    if (on)
    {
        g.setColour (col::orange);
        g.fillRoundedRectangle (juce::Rectangle<float> (16, 2.2f).withCentre ({ face.getCentreX(), face.getY() + 4 }), 1);
    }
}

void WoodLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    auto r = b.getLocalBounds();
    const bool on = b.getToggleState();
    if (b.getName() == "tips")
    {
        g.setColour (col::muted); g.setFont (uiFont (10));
        g.drawText (b.getButtonText(), r.withTrimmedLeft (18), juce::Justification::centredLeft);
        return;
    }
    if (on) r.translate (0, 2);
    g.setFont (uiFont (10));
    const auto text = b.getButtonText();
    const auto sw = b.getProperties()["swatch"];
    const int tw = juce::GlyphArrangement::getStringWidthInt (g.getCurrentFont(), text);
    int x = r.getCentreX() - tw / 2;
    if (! sw.isVoid())
    {
        x = r.getCentreX() - (tw + 18) / 2;
        auto s = juce::Rectangle<float> ((float) x, (float) r.getCentreY() - 6, 12, 12);
        g.setColour (col::swatch[(int) sw]); g.fillRoundedRectangle (s, 3);
        g.setColour (juce::Colours::black.withAlpha (0.18f));
        for (float k = s.getX() + 2; k < s.getRight(); k += 3) g.drawLine (k, s.getY() + 1, k + 1.5f, s.getBottom() - 1, 0.8f);
        g.setColour (juce::Colours::black.withAlpha (0.25f)); g.drawRoundedRectangle (s, 3, 1);
        x += 18;
    }
    g.setColour (on ? juce::Colour (0xff1d1c1a) : juce::Colour (0xff4a4740));
    g.drawText (text, x, r.getY(), tw + 4, r.getHeight(), juce::Justification::centredLeft);
}

void WoodLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int w, int h)
{
    g.setColour (juce::Colour (0xff14181b)); g.fillRoundedRectangle (0, 0, (float) w, (float) h, 6);
    g.setColour (juce::Colour (0xff3c4a45)); g.drawRoundedRectangle (0.5f, 0.5f, (float) w - 1, (float) h - 1, 6, 1);
    juce::AttributedString s; s.setText (text); s.setFont (uiFont (12, false)); s.setColour (juce::Colour (0xffe7f5ef));
    s.setWordWrap (juce::AttributedString::byWord);
    juce::TextLayout tl; tl.createLayout (s, (float) w - 16);
    tl.draw (g, { 8, 6, (float) w - 16, (float) h - 12 });
}

juce::Rectangle<int> WoodLookAndFeel::getTooltipBounds (const juce::String& text, juce::Point<int> pos, juce::Rectangle<int> parent)
{
    juce::AttributedString s; s.setText (text); s.setFont (uiFont (12, false)); s.setWordWrap (juce::AttributedString::byWord);
    juce::TextLayout tl; tl.createLayout (s, 230);
    const int w = (int) tl.getWidth() + 18, h = (int) tl.getHeight() + 14;
    return juce::Rectangle<int> (pos.x + 12, pos.y - h - 10, w, h).constrainedWithin (parent);
}

// ---------------------------------------------------------------- Content
static const char* kWoodNames[] = { "SPRUCE", "MAHOGANY", "ROSEWOOD" };
static const char* kWoodTips[]  = { "Spruce: bright, open, airy", "Mahogany: warm, thick low-mids", "Rosewood: deep bass, rich mids" };
static const char* kStNames[]   = { "MONO", "X-Y", "SPACED" };
static const char* kStTips[]    = { "Single centered mic", "Two mics at one point, 90 degrees. Mono-safe", "Two mics apart. Wider, check mono" };

WoodBodyContent::WoodBodyContent (WoodBodyOneProcessor& p) : proc (p)
{
    setLookAndFeel (&lnf);

    wood.setName ("big");
    wood.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    wood.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    wood.setMouseDragSensitivity (300);
    wood.setDoubleClickReturnValue (true, 40.0);
    addAndMakeVisible (wood);
    woodAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "wood", wood);

    output.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    output.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    output.setDoubleClickReturnValue (true, 0.0);
    addAndMakeVisible (output);
    outAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "output", output);

    for (int i = 0; i < 3; ++i)
    {
        woodBtn[i].setButtonText (kWoodNames[i]);
        woodBtn[i].getProperties().set ("swatch", i);
        woodBtn[i].onClick = [this, i] {
            if (auto* prm = proc.apvts.getParameter ("woodType")) prm->setValueNotifyingHost (prm->convertTo0to1 ((float) i));
            syncButtons(); };
        addAndMakeVisible (woodBtn[i]);

        stBtn[i].setButtonText (kStNames[i]);
        stBtn[i].onClick = [this, i] {
            if (auto* prm = proc.apvts.getParameter ("stereo")) prm->setValueNotifyingHost (prm->convertTo0to1 ((float) i));
            syncButtons(); };
        addAndMakeVisible (stBtn[i]);
    }

    tipsBtn.setName ("tips");
    tipsBtn.setButtonText ("TIPS");
    tipsBtn.onClick = [this] { proc.tipsOn = ! proc.tipsOn.load(); applyTips(); };
    addAndMakeVisible (tipsBtn);

    applyTips();
    syncButtons();
    startTimerHz (30);
}

WoodBodyContent::~WoodBodyContent() { setLookAndFeel (nullptr); }

void WoodBodyContent::applyTips()
{
    const bool on = proc.tipsOn.load();
    tipsBtn.setToggleState (on, juce::dontSendNotification);
    auto tip = [on] (juce::SettableTooltipClient& c, const char* t) { c.setTooltip (on ? juce::String (t) : juce::String()); };
    tip (wood, "One knob does it all: body IR, warmth, woody saturation, de-harsh and attack. Drag up/down, double-click to reset");
    tip (output, "Output level. Double-click to reset to 0 dB");
    tip (tipsBtn, "Turn hover tips on or off");
    for (int i = 0; i < 3; ++i) { tip (woodBtn[i], kWoodTips[i]); tip (stBtn[i], kStTips[i]); }
}

void WoodBodyContent::syncButtons()
{
    const int wt = (int) proc.apvts.getRawParameterValue ("woodType")->load();
    const int st = (int) proc.apvts.getRawParameterValue ("stereo")->load();
    for (int i = 0; i < 3; ++i)
    {
        woodBtn[i].setToggleState (i == wt, juce::dontSendNotification);
        stBtn[i].setToggleState (i == st, juce::dontSendNotification);
    }
}

void WoodBodyContent::timerCallback()
{
    syncButtons();
    meterIn  = juce::jmax (proc.meterIn.load(),  meterIn * 0.85f);
    meterOut = juce::jmax (proc.meterOut.load(), meterOut * 0.85f);
    repaint();
}

void WoodBodyContent::resized()
{
    tipsBtn.setBounds (370, 20, 52, 22);
    displayArea = { 44, 76, 372, 122 };
    wood.setBounds (95, 200, 270, 262);
    for (int i = 0; i < 3; ++i) woodBtn[i].setBounds (44 + i * 126, 530, 120, 34);
    output.setBounds (44, 578, 40, 40);
    for (int i = 0; i < 3; ++i) stBtn[i].setBounds (236 + i * 61, 586, 57, 28);
}

static void drawWoodCheek (juce::Graphics& g, juce::Rectangle<float> r, bool left)
{
    g.setGradientFill (juce::ColourGradient (left ? col::woodA : col::woodC, r.getX(), 0, left ? col::woodC : col::woodA, r.getRight(), 0, false));
    g.fillRect (r);
    juce::Random rnd (left ? 7 : 11);
    for (float y = r.getY(); y < r.getBottom(); y += 2.0f + rnd.nextFloat() * 5.0f)
    {
        g.setColour (juce::Colours::black.withAlpha (0.06f + rnd.nextFloat() * 0.12f));
        g.drawLine (r.getX(), y, r.getRight(), y + (rnd.nextFloat() - 0.5f) * 3.0f, 0.8f);
    }
    g.setColour (col::woodB.withAlpha (0.35f));
    g.fillRect (r.reduced (r.getWidth() * 0.3f, 0).withWidth (r.getWidth() * 0.25f));
    g.setColour (juce::Colour (0xff2b180b));
    g.drawLine (left ? r.getRight() : r.getX(), r.getY(), left ? r.getRight() : r.getX(), r.getBottom(), 1.0f);
}

static void drawScrew (juce::Graphics& g, float x, float y)
{
    auto r = juce::Rectangle<float> (11, 11).withCentre ({ x, y });
    g.setGradientFill (juce::ColourGradient (juce::Colours::white, r.getX() + 3, r.getY() + 3, juce::Colour (0xff9a958c), r.getRight(), r.getBottom(), true));
    g.fillEllipse (r);
    g.setColour (juce::Colour (0xff8a857c)); g.drawEllipse (r, 1);
    g.setColour (juce::Colour (0xff6c675f));
    g.drawLine (juce::Line<float> (r.getCentre().getPointOnCircumference (3.5f, 2.2f), r.getCentre().getPointOnCircumference (3.5f, 5.34f)), 1.0f);
}

void WoodBodyContent::paint (juce::Graphics& g)
{
    auto all = getLocalBounds().toFloat();
    juce::Path shape; shape.addRoundedRectangle (all, 14);
    g.reduceClipRegion (shape);

    g.setGradientFill (juce::ColourGradient (col::panelTop, 0, 0, col::panelBot, 0, (float) H, false));
    g.fillAll();
    drawWoodCheek (g, { 0, 0, 28, (float) H }, true);
    drawWoodCheek (g, { (float) W - 28, 0, 28, (float) H }, false);
    drawScrew (g, 14, 18); drawScrew (g, (float) W - 14, 18);
    drawScrew (g, 14, (float) H - 18); drawScrew (g, (float) W - 14, (float) H - 18);

    // header
    g.setColour (col::ink); g.setFont (uiFont (20));
    g.drawText ("WOOD", 44, 14, 70, 26, juce::Justification::centredLeft);
    g.setColour (col::walnut); g.drawText (juce::String::fromUTF8 ("\xc2\xb7"), 104, 14, 12, 26, juce::Justification::centred);
    g.setColour (col::ink); g.drawText ("BODY", 116, 14, 70, 26, juce::Justification::centredLeft);
    g.setColour (col::walnut); g.drawText ("ONE", 178, 14, 60, 26, juce::Justification::centredLeft);
    g.setColour (col::muted); g.setFont (uiFont (8.5f, false));
    g.drawText ("ONE-KNOB ACOUSTIC ENHANCER", 44, 40, 260, 12, juce::Justification::centredLeft);
    {
        juce::ColourGradient tg (col::woodC, 28, 0, col::woodC, (float) W - 28, 0, false);
        tg.addColour (0.5, juce::Colour (0xff9a6236));
        g.setGradientFill (tg); g.fillRect (28.0f, 60.0f, (float) W - 56, 3.0f);
    }

    // display
    auto d = displayArea.toFloat();
    g.setGradientFill (juce::ColourGradient (col::lcdTop, 0, d.getY(), col::lcdBot, 0, d.getBottom(), false));
    g.fillRoundedRectangle (d, 10);
    g.setColour (juce::Colour (0xff05080a)); g.drawRoundedRectangle (d, 10, 1);
    g.setColour (juce::Colours::black.withAlpha (0.35f)); g.drawRoundedRectangle (d.reduced (1.5f), 9, 2);

    const float w01 = proc.apvts.getRawParameterValue ("wood")->load() / 100.0f;
    const int wt = (int) proc.apvts.getRawParameterValue ("woodType")->load();
    const auto m = WoodBodyOneProcessor::computeMacro (w01, wt);
    const char* stage = w01 < 0.12f ? "BYPASS-ISH" : w01 < 0.35f ? "SUBTLE" : w01 < 0.6f ? "WARM" : w01 < 0.85f ? "THICK" : "HUGE";

    g.setFont (monoFont (10));
    g.setColour (col::glow.withAlpha (0.55f));
    g.drawText ("MACRO", (int) d.getX() + 12, (int) d.getY() + 8, 100, 12, juce::Justification::centredLeft);
    g.setColour (col::glow);
    g.drawText (juce::String (stage) + " | " + kWoodNames[wt], (int) d.getX() + 120, (int) d.getY() + 8, (int) d.getWidth() - 132, 12, juce::Justification::centredRight);

    const char* rows[] = { "BODY IR", "WARMTH", "GRAIN", "DE-HARSH", "ATTACK" };
    const float vals[] = { m.body, m.warmthDb / 6.0f, m.grain, m.deharshDb / 5.0f, m.attack };
    const juce::String txt[] = { juce::String (juce::roundToInt (m.body * 100)) + "%",
                                 "+" + juce::String (m.warmthDb, 1) + "dB",
                                 juce::String (juce::roundToInt (m.grain * 100)) + "%",
                                 "-" + juce::String (m.deharshDb, 1) + "dB",
                                 juce::String (juce::roundToInt (m.attack * 100)) + "%" };
    for (int i = 0; i < 5; ++i)
    {
        const float y = d.getY() + 28 + (float) i * 17.5f;
        g.setColour (col::glow.withAlpha (0.7f));
        g.drawText (rows[i], (int) d.getX() + 12, (int) y - 1, 70, 12, juce::Justification::centredLeft);
        auto bar = juce::Rectangle<float> (d.getX() + 84, y + 2, d.getWidth() - 84 - 66, 6);
        g.setColour (juce::Colour (0xff0a0e11)); g.fillRoundedRectangle (bar, 3);
        auto fill = bar.withWidth (bar.getWidth() * juce::jlimit (0.0f, 1.0f, vals[i]));
        g.setColour (col::glow.withAlpha (0.25f)); g.fillRoundedRectangle (fill.expanded (0, 1.5f), 3);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4fb595), fill.getX(), 0, col::glow, fill.getRight(), 0, false));
        g.fillRoundedRectangle (fill, 3);
        g.setColour (col::glow);
        g.drawText (txt[i], (int) bar.getRight() + 4, (int) y - 1, 52, 12, juce::Justification::centredRight);
    }

    // wood label / value / hint
    g.setColour (col::ink); g.setFont (uiFont (15));
    g.drawText ("W  O  O  D", 0, 462, W, 18, juce::Justification::centred);
    g.setColour (juce::Colour (0xff5b574f)); g.setFont (monoFont (13));
    g.drawText (juce::String (juce::roundToInt (w01 * 100)) + "%", 0, 482, W, 16, juce::Justification::centred);
    const char* hint = w01 < 0.12f ? "Almost untouched" : w01 < 0.35f ? "A touch of body, harshness tamed"
                     : w01 < 0.6f ? "Fuller body, woody warmth" : w01 < 0.85f ? "Thick body + grain, attack softens"
                     : "Maximum body and saturation";
    g.setColour (col::muted); g.setFont (uiFont (11, false));
    g.drawText (hint, 0, 502, W, 14, juce::Justification::centred);

    // output label + meters
    const float od = proc.apvts.getRawParameterValue ("output")->load();
    g.setColour (col::ink); g.setFont (uiFont (9));
    g.drawText ("OUTPUT", 92, 584, 70, 12, juce::Justification::centredLeft);
    g.setColour (juce::Colour (0xff5b574f)); g.setFont (monoFont (11));
    g.drawText ((od > 0 ? "+" : "") + juce::String (od, 1) + " dB", 92, 600, 80, 14, juce::Justification::centredLeft);
    auto meter = [&] (float x, float v)
    {
        auto r = juce::Rectangle<float> (x, 584, 6, 30);
        g.setColour (juce::Colour (0xffc9c5bc)); g.fillRoundedRectangle (r, 2);
        const float db = juce::jlimit (-60.0f, 0.0f, juce::Decibels::gainToDecibels (v + 1.0e-9f));
        auto f = r.withTop (r.getBottom() - r.getHeight() * (db + 60.0f) / 60.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4fb595), 0, r.getBottom(), juce::Colour (0xffef6b4a), 0, r.getY(), false));
        g.fillRoundedRectangle (f, 2);
    };
    meter (178, meterIn); meter (188, meterOut);
    g.setColour (col::muted); g.setFont (uiFont (7, false));
    g.drawText ("IN OUT", 170, 616, 34, 8, juce::Justification::centred);

    // footer
    g.setFont (uiFont (10, false));
    juce::AttributedString foot;
    foot.append ("Developed by ", uiFont (10, false), col::muted);
    foot.append ("Ewin", uiFont (10, true), col::ink);
    foot.append (juce::String::fromUTF8 (" \xc2\xb7 Made with Claude"), uiFont (10, false), col::muted);
    foot.setJustification (juce::Justification::centred);
    foot.draw (g, { 28, (float) H - 18, (float) W - 56, 14 });

    // resize grip hint
    g.setColour (col::muted.withAlpha (0.6f));
    for (int i = 0; i < 3; ++i) g.drawLine ((float) W - 34 - i * 4, (float) H - 6, (float) W - 30, (float) H - 10 - i * 4, 1.0f);

    g.setColour (juce::Colour (0xffbdb8ae));
    g.drawRoundedRectangle (all.reduced (0.5f), 14, 1);
}

// ---------------------------------------------------------------- Editor
WoodBodyOneEditor::WoodBodyOneEditor (WoodBodyOneProcessor& p) : AudioProcessorEditor (&p), content (p)
{
    addAndMakeVisible (content);
    content.setBounds (0, 0, WoodBodyContent::W, WoodBodyContent::H);
    tooltip.setLookAndFeel (&content.getLookAndFeel());
    setResizable (true, true);
    setResizeLimits (WoodBodyContent::W * 3 / 4, WoodBodyContent::H * 3 / 4, WoodBodyContent::W * 2, WoodBodyContent::H * 2);
    getConstrainer()->setFixedAspectRatio ((double) WoodBodyContent::W / WoodBodyContent::H);
    setSize (WoodBodyContent::W, WoodBodyContent::H);
}

void WoodBodyOneEditor::resized()
{
    const float s = (float) getWidth() / (float) WoodBodyContent::W;
    content.setTransform (juce::AffineTransform::scale (s));
}
