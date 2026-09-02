#pragma once
#include "palette.h"
#include <vector>
#include <array>


namespace vrui {

	class label_palette : public palette {

	public:
		label_palette();

		/**
			builds a grid of spheres with the ids 0 to width*height-1
			@param PALETTE_COLOR_MAPPING : sphere colors
		*/
		void build(const std::vector<cgv::rgba>& PALETTE_COLOR_MAPPING, int width = 5, int height = 5, const PaletteObject shape = PaletteObject::PO_BOX_PLANE);

		/// returns the global id of the instance-counter display button (row 5, center), or -1 if not yet built
		int get_instance_display_id() const { return instance_display_button_id; }

		/// returns the global palette id of right-side shortcut button at position_in_group \p pos (0-12), or -1 if invalid
		int get_shortcut_id(int pos) const { return (pos >= 0 && pos < 13) ? shortcut_ids[pos] : -1; }

		/// position_in_group of the panel toggle buttons inside POG_LEFT_TOOLBAR
		static constexpr int first_toggle_position = 13;
		/// number of buttons on the bottom toggle panel
		static constexpr int num_toggles = 4;
		/// returns the global palette id of panel toggle button \p i
		/// (0 = left panel, 1 = lock side panels, 2 = right panel, 3 = hand/floating mode)
		int get_toggle_id(int i) const { return (i >= 0 && i < num_toggles) ? toggle_ids[i] : -1; }

	private:
		int instance_display_button_id = -1;
		std::array<int, 13> shortcut_ids{{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};
		std::array<int, 4> toggle_ids{{-1,-1,-1,-1}};
	};

}