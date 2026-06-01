#include "controller_labels.h"

pct::controller_labels::controller_labels() : label_cs(CoordinateSystem::CS_LAB){
	for (int i = 0; i < CLP_NUM_LABEL_PLACEMENTS; ++i) {
		active_labels[i] = -1;
		reference_positions[i] = vec3(0);
		reference_orientations[i] = quat(1.f, 0.f, 0.f, 0.f);
	}

	reference_positions[CLP_GRIP] = vec3(-0.0223, -0.0144, 0.0855);
	reference_orientations[CLP_GRIP] = quat(vec3(0, 1, 0), -1.5f);
	reference_scale[CLP_GRIP] = 0.2f;

	float trackpad_scale = 0.125f;

	// CLP_SIDE = A/X button — on the -X axis ball (inner side of right controller)
	reference_positions[CLP_SIDE] = vec3(-0.05, 0.0, 0.012);
	reference_orientations[CLP_SIDE] = quat(vec3(1, 0, 0), -acos(0.f));
	reference_scale[CLP_SIDE] = trackpad_scale;

	reference_positions[CLP_TRACKPAD_UP] = vec3(0.0, 0.05, -0.012);
	reference_orientations[CLP_TRACKPAD_UP] = quat(vec3(1, 0, 0), -acos(0.f));
	reference_scale[CLP_TRACKPAD_UP] = trackpad_scale;

	reference_positions[CLP_TRACKPAD_DOWN] = vec3(0.0, 0.05, 0.012);
	reference_orientations[CLP_TRACKPAD_DOWN] = quat(vec3(1, 0, 0), -acos(0.f));
	reference_scale[CLP_TRACKPAD_DOWN] = trackpad_scale;
	float trackpad_horizontal_center_line = 0.0;
	float trackpad_left_right_spread = 0.03;
	reference_positions[CLP_TRACKPAD_LEFT] = vec3(-trackpad_left_right_spread, 0.05, trackpad_horizontal_center_line);
	reference_orientations[CLP_TRACKPAD_LEFT] = quat(vec3(1, 0, 0), -acos(0.f));
	reference_scale[CLP_TRACKPAD_LEFT] = trackpad_scale;

	reference_positions[CLP_TRACKPAD_RIGHT] = vec3(trackpad_left_right_spread, 0.05, trackpad_horizontal_center_line);
	reference_orientations[CLP_TRACKPAD_RIGHT] = quat(vec3(1, 0, 0), -acos(0.f));
	reference_scale[CLP_TRACKPAD_RIGHT] = trackpad_scale;

	reference_positions[CLP_TRACKPAD_CENTER] = vec3(0.0, 0.05, -0.025);
	reference_orientations[CLP_TRACKPAD_CENTER] = quat(vec3(1, 0, 0), -acos(0.f));
	reference_scale[CLP_TRACKPAD_CENTER] = trackpad_scale;

	// CLP_MENU_BUTTON = B/Y button — on the -X axis ball (inner side of right controller), offset from A/X
	reference_positions[CLP_MENU_BUTTON] = vec3(-0.05, 0.0, -0.012);
	reference_orientations[CLP_MENU_BUTTON] = quat(vec3(1, 0, 0), -acos(0.f));
	reference_scale[CLP_MENU_BUTTON] = trackpad_scale;

	// CLP_TRIGGER = trigger button — below and in front, same orientation/size as grip
	reference_positions[CLP_TRIGGER] = vec3(-0.02, -0.045, -0.02);
	reference_orientations[CLP_TRIGGER] = quat(vec3(0, 1, 0), -1.5f);
	reference_scale[CLP_TRIGGER] = 0.2f;
}

void pct::controller_labels::set_active_profile(active_labels_array& profile)
{
	memcpy(active_labels.data(), profile.data(), active_labels.size());
}

void pct::controller_labels::set_active(controller_label_placement location, int label_idx)
{
	active_labels[location] = label_idx;
}

void pct::controller_labels::set_controller_label(controller_label_placement label_placement, int variant_ix, const std::string& text, const rgba& bg_clr)
{
	if (!check_address(label_placement, variant_ix)) {
		throw std::runtime_error("attempted to write to unallocated label");
	}

	auto& label_id = label_ids[label_placement][variant_ix];
	if (label_id == -1) {
		label_id = this->add_label(text, bg_clr);
	}
	else {
		this->set_label(label_id, text, bg_clr);
	}
	
	LabelAlignment l_align = LA_CENTER;
	
	this->place_label(label_id,
		reference_positions[label_placement], reference_orientations[label_placement], label_cs, l_align, reference_scale[label_placement]);
	this->hide_label(label_id);
}

void pct::controller_labels::update_placement(controller_label_placement location, int label_idx)
{
	auto label_id = label_ids[location][label_idx];
	LabelAlignment l_align = LA_CENTER;
	this->place_label(label_id,
		reference_positions[location], reference_orientations[location], label_cs, l_align, reference_scale[location]);
}

bool pct::controller_labels::check_address(controller_label_placement label_placement, int variant_ix) const
{
	return (label_placement >= 0 && label_placement < label_ids.size() &&
		variant_ix >= 0 && variant_ix < label_ids[label_placement].size());
}

int pct::controller_labels::add_variant(controller_label_placement label_placement, const std::string& text, const rgba& bg_clr)
{
	int new_variant_ix = label_ids[label_placement].size();
	label_ids[label_placement].push_back(-1);

	set_controller_label(label_placement, new_variant_ix, text, bg_clr);
	return new_variant_ix;
}

void pct::controller_labels::set_label_color(controller_label_placement location, int label_idx, const rgba& bg_clr)
{
	auto label_id = label_ids[location][label_idx];
	set_controller_label(location, label_idx, get_label_text(label_id), bg_clr);
}



void pct::controller_labels::draw(cgv::render::context& ctx)
{
	for (int i = 0; i < CLP_NUM_LABEL_PLACEMENTS; ++i) {
		if (active_labels[i] != -1) {
			int li = label_ids[i][active_labels[i]];
			if (check_handle(li)) {
				if (i == CLP_GRIP || i == CLP_SIDE || i == CLP_MENU_BUTTON || i == CLP_TRIGGER)
					this->draw_mirrored(ctx, li, (controller_label_placement)i);
				else
					vr_labels::draw(ctx, li);
			}
		}
	}
}

void pct::controller_labels::draw_mirrored(cgv::render::context& ctx, int li, controller_label_placement placement)
{
	// Draw label mirrored on left controller: negate X position
	// For grip/trigger style labels (Y-axis rotation), also conjugate orientation
	// For trackpad-style labels (X-axis rotation), keep orientation as-is
	vec3 pos;
	quat ori;
	if (label_cs == CoordinateSystem::CS_RIGHT_CONTROLLER) {
		pos = reference_positions[placement];
		ori = reference_orientations[placement];
	} else {
		pos = vec3(-reference_positions[placement].x(), reference_positions[placement].y(), reference_positions[placement].z());
		// Only conjugate for grip/trigger (Y-axis oriented labels)
		if (placement == CLP_GRIP || placement == CLP_TRIGGER)
			ori = reference_orientations[placement].conj();
		else
			ori = reference_orientations[placement];
	}
	draw_multiple(ctx, li, &pos, &ori, 1);
}
