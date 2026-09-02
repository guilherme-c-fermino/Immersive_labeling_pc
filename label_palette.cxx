#include "label_palette.h"

namespace vrui {
	label_palette::label_palette()
	{
	}

	void label_palette::build(const std::vector<cgv::rgba>& PALETTE_COLOR_MAPPING, int width, int height, const PaletteObject shape)
	{
		sphere_style().radius = 0.02f;
		box_style().default_extent = cgv::vec3(0.04, 0.04, 0.04);
		wire_box_style().default_extent = cgv::vec3(0.04, 0.04, 0.04);
		box_plane_style().default_extent = cgv::vec3(0.04, 0.04, 0.04);

		// 25 positions for labels = 0.05
		static double constexpr step_width = 0.10;

		// sphere grid (main semantic palette)
		uintptr_t num_center_spheres = 0;
		int col_i = 0;
		int off = width / 2;
		for (int iz = height; iz > 0; iz--) {
			for (int ix = -off; ix < width-off; ix++) {
				int id = this->add_object(
					shape,
					cgv::vec3(ix * step_width, 0.1, -iz * step_width),
					PALETTE_COLOR_MAPPING[col_i++]);
			}
		}

		// Right-side shortcut buttons (12 total, position_in_group 0-11 within POG_LEFT_TOOLBAR)
		static const cgv::rgba placeholder_color(0.0f, 0.0f, 0.0f, 0.5f);
		const int first_placeholder_x = off + 1; // immediately to the right of the main grid
		int sp = 0; // shortcut position counter, fills shortcut_ids in build order

		// Row 1 (pos 0-1): paint-mode +SIZE / -SIZE (labels set dynamically by update_interaction_mode)
		for (int c = 0; c < 2; ++c)
			shortcut_ids[sp++] = this->add_object(shape, cgv::vec3((first_placeholder_x + c) * step_width, 0.1, -6 * step_width), placeholder_color, PaletteObjectGroup::POG_LEFT_TOOLBAR);

		// Row 2 (pos 2-3): paint-mode IN / OUT (labels set dynamically by update_interaction_mode)
		for (int c = 0; c < 2; ++c)
			shortcut_ids[sp++] = this->add_object(shape, cgv::vec3((first_placeholder_x + c) * step_width, 0.1, -5 * step_width), placeholder_color, PaletteObjectGroup::POG_LEFT_TOOLBAR);

		// Row 3 (pos 4): single centered UNDO button
		{
			int id = this->add_object(shape, cgv::vec3((first_placeholder_x + 0.5) * step_width, 0.1, -4 * step_width), placeholder_color, PaletteObjectGroup::POG_LEFT_TOOLBAR);
			set_label_text(id, "UNDO");
			shortcut_ids[sp++] = id;
		}

		// Row 4 (pos 5-6): SEM view / INST view
		{
			int id = this->add_object(shape, cgv::vec3(first_placeholder_x * step_width, 0.1, -3 * step_width), placeholder_color, PaletteObjectGroup::POG_LEFT_TOOLBAR);
			set_label_text(id, "SEM");
			shortcut_ids[sp++] = id;
			id = this->add_object(shape, cgv::vec3((first_placeholder_x + 1) * step_width, 0.1, -3 * step_width), placeholder_color, PaletteObjectGroup::POG_LEFT_TOOLBAR);
			set_label_text(id, "INST");
			shortcut_ids[sp++] = id;
		}

		// Row 5 (pos 7-9): +COUNT / instance display / -COUNT
		{
			int id = this->add_object(shape, cgv::vec3(first_placeholder_x * step_width, 0.1, -2 * step_width), placeholder_color, PaletteObjectGroup::POG_LEFT_TOOLBAR);
			set_label_text(id, "+CNT");
			shortcut_ids[sp++] = id;
			id = this->add_object(shape, cgv::vec3((first_placeholder_x + 0.5) * step_width, 0.1, -2 * step_width), placeholder_color, PaletteObjectGroup::POG_LEFT_TOOLBAR);
			set_label_text(id, "1"); // updated at runtime with current instance counter
			instance_display_button_id = id;
			shortcut_ids[sp++] = id;
			id = this->add_object(shape, cgv::vec3((first_placeholder_x + 1) * step_width, 0.1, -2 * step_width), placeholder_color, PaletteObjectGroup::POG_LEFT_TOOLBAR);
			set_label_text(id, "-CNT");
			shortcut_ids[sp++] = id;
		}

		// Row 6 (pos 10-11): placeholders (empty/unused in all modes)
		for (int c = 0; c < 2; ++c)
			shortcut_ids[sp++] = this->add_object(shape, cgv::vec3((first_placeholder_x + c) * step_width, 0.1, -1 * step_width), placeholder_color, PaletteObjectGroup::POG_LEFT_TOOLBAR);

		// Row 1 center (pos 12): CP mode param-cycle toggle (hidden by default, shown only in LABELING_3)
		{
			int id = this->add_object(shape, cgv::vec3((first_placeholder_x + 0.5) * step_width, 0.1, -6 * step_width), placeholder_color, PaletteObjectGroup::POG_LEFT_TOOLBAR);
			shortcut_ids[12] = id;
			this->set_object_visibility(id, false);
		}

		// Panel toggles (pos 13-15): shown on their own small panel. Left switches the colour
		// panel, middle locks both side panels in place, right switches the function button panel.
		for (int c = 0; c < 3; ++c) {
			toggle_ids[c] = this->add_object(shape,
				cgv::vec3((first_placeholder_x + c) * step_width, 0.1, 1 * step_width),
				placeholder_color, PaletteObjectGroup::POG_LEFT_TOOLBAR);
		}
		// Pos 16: one row further up (smaller z maps to higher on the upright panel), switches
		// between the hand mounted palette and the floating panels.
		toggle_ids[3] = this->add_object(shape,
			cgv::vec3((first_placeholder_x + 1) * step_width, 0.1, 0 * step_width),
			placeholder_color, PaletteObjectGroup::POG_LEFT_TOOLBAR);

		set_top_toolbar_visibility(true);

		// text for palette
		set_label_text(0, "delete");
		set_label_text(1, "clear label");
	
		set_palette_changed();
	}


}