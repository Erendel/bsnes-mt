/* MT. */
#include "bsnes-mt/translations.h"

namespace bmt = bsnesMt::translations;
/* /MT. */

auto VideoSettings::create() -> void {
	setCollapsible();
	setVisible(false);

	char colon = ':';

	colorAdjustmentLabel.setFont(Font().setBold()).setText(bmt::get("Settings.Video.ColorAdjustment").data());

	colorLayout.setSize({3, 3});
	colorLayout.column(0).setAlignment(1.0);

	luminanceLabel.setText({bmt::get("Settings.Video.Luminance").data(), colon});
	luminanceValue.setAlignment(0.5);

	luminanceSlider.setLength(101).setPosition(settings.video.luminance)
		.onChange([&] {
			string value = {luminanceSlider.position(), "%"};
			settings.video.luminance = value.natural();
			luminanceValue.setText(value);
			program.updateVideoPalette();
		})
		.doChange();

	saturationLabel.setText({bmt::get("Settings.Video.Saturation").data(), colon});
	saturationValue.setAlignment(0.5);

	saturationSlider.setLength(201).setPosition(settings.video.saturation)
		.onChange([&] {
			string value = {saturationSlider.position(), "%"};
			settings.video.saturation = value.natural();
			saturationValue.setText(value);
			program.updateVideoPalette();
		})
		.doChange();

	gammaLabel.setText({bmt::get("Settings.Video.Gamma").data(), colon});
	gammaValue.setAlignment(0.5);

	gammaSlider.setLength(101).setPosition(settings.video.gamma - 100)
		.onChange([&] {
			string value = {100 + gammaSlider.position(), "%"};
			settings.video.gamma = value.natural();
			gammaValue.setText(value);
			program.updateVideoPalette();
		})
		.doChange();

	dimmingOption.setText(bmt::get("Settings.Video.DimVideoWhenIdle").data())
		.setToolTip(bmt::get("Settings.Video.DimVideoWhenIdle.tooltip").data())
		.setChecked(settings.video.dimming)
		.onToggle([&] {
			settings.video.dimming = dimmingOption.checked();
		});

	snowOption.setText(bmt::get("Settings.Video.DrawSnowEffectWhenIdle").data())
		.setChecked(settings.video.snow)
		.onToggle([&] {
			settings.video.snow = snowOption.checked();
			presentation.updateProgramIcon();
		});
}