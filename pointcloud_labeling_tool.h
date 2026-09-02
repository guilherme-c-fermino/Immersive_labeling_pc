#pragma once

#include <cgv/base/node.h>
#include <cgv/math/fvec.h>
#include <cgv/media/color.h>
#include <cgv/math/functions.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/data/data_view.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/texture.h>
#include <cgv/utils/statistics.h>
#include <cgv_gl/point_renderer.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv_gl/surfel_renderer.h>
#include <cgv_gl/clod_point_renderer.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/arrow_renderer.h>
#include <cgv_gl/box_wire_renderer.h>
#include <cgv_gl/attribute_array_manager.h>
#include <cgv/utils/stopwatch.h>
#include <cgv_gl/gl/mesh_render_info.h>

#include <point_cloud.h>
#include <concurrency.h>
#include <octree.h>
#include "chunks.h"
#include "history.h"
#include "label_shader_manager.h"
#include "intersection.h"
#include "vr_labels.h"
#include "world.h"
#include "selection_tool.h"
#include "statistics_collection.h"
#include "pointcloud_transformation_state.h"
#include "point_cloud_registration_tool.h"
#include "point_labels.h"
#include "clod_reduction_state.h"
#include "label_palette.h"
#include "point_cloud_palette.h"
#include "bitfield_control_provider.h"
#include "point_cloud_clipboard.h"
#include "controller_labels.h"

#include <map>
#include <string>

#include <ann_tree.h>

#include <string>
#include <sstream>
#include <mutex>
#include <future>
#include <atomic>
#include <array>
#include <set>
#include <io.h>
#include <fstream>
#include <iostream>

#include <libs/point_cloud/point_cloud_provider.h>

// these are the vr specific headers
#include <vr/vr_driver.h>
#include <cg_vr/vr_server.h>
#include <vr_view_interactor.h>
#include <vr_render_helpers.h>
#include <cg_vr/vr_events.h>

#include "point_cloud_server.h"
#include "clod_parameter_controller.h"
#include "waypoint_navigation.h"

#include "lib_begin.h"


enum class ConfigModeTools {
	CMT_CLODParameters,
	CMT_PointSpacingProbe,
	CMT_NUM_ENUMS,
};

enum class LabelOperation {
	REPLACE = 0,
	OR = 1,
	AND = 2,
	NONE = -1
};


enum InteractionMode {
	TELEPORT = 0,
	LABELING = 1,
	LABELING_2 = 2,
	LABELING_3 = 3,
	CONFIG = 4,
	NUM_OF_INTERACTIONS
};

enum PalettePosition {
	HEADON = 0,
	RIGHTHAND = 1,
	LEFTHAND = 2,
	/// palette floats in front of the head: colours on the left of the view, function buttons on the right
	HEAD_SIDES = 3,
	NUM_OF_PALETTE_POSITIONS
};


inline selection_shape to_selection_shape(const vrui::PaletteObject po) {
	if (po == vrui::PO_BOX_FRAME)
		return selection_shape::SS_CUBOID;
	return (selection_shape) po;
}

inline vrui::PaletteObject to_PaletteObject(const selection_shape ss) {
	return (vrui::PaletteObject)ss;
}


class pointcloud_labeling_tool;


enum point_attributes {
	PA_LABELS = 1,
	PA_POSITIONS = 2,
	PA_COLORS = 4,
	PA_LODS = 8,
	PA_ALL = -1
};

enum pallete_tool {
	PT_BRUSH = 0,
	PT_SELECTION = 1,
	NUM_PALLETE_TOOLS
};

class pointcloud_labeling_tool :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider
{
public:
	//types in cgv namespace
	using vec3 = cgv::vec3;
	using vec4 = cgv::vec4;
	using mat3 = cgv::mat3;
	using mat4 = cgv::mat4;
	using quat = cgv::quat;
	using rgb = cgv::rgb;
	using rgba = cgv::rgba;

	//type alias for point format used by the clod renderer
	using LODPoint = cgv::render::clod_point_renderer::Point;

	pointcloud_labeling_tool();

	~pointcloud_labeling_tool();

	/// overload to return the type name of this object. By default the type interface is queried over get_type.
	std::string get_type_name() const { return "pointcloud_cleaning_tool"; }

	bool self_reflect(cgv::reflect::reflection_handler& rh) override;

	void on_set(void* member_ptr) override;
	
	void on_register() override;
	
	void unregister() override;

	bool init(cgv::render::context& ctx) override;

	void init_frame(cgv::render::context& ctx) override;
	
	void draw(cgv::render::context& ctx) override;
	///
	void finish_draw(cgv::render::context& ctx) override;

	void on_device_change(void* kit_handle, bool attach);
	/// create several modes for easy picking
	void render_candidate_modes(cgv::render::context& ctx);
	/// helper functions for gui rendering 
	void render_a_handhold_arrow(cgv::render::context& ctx, rgb c, float r);
	/// creates a palette for label picking
	void build_palette();
	/// draw a sphere on the right hand
	void render_palette_sphere_on_rhand(cgv::render::context& ctx, const rgba& color);
	/// enable or disable the palettes toolbar
	void set_palette_toolbar_visibilty(bool visibility);
	/// draw a plane on the right hand
	void render_palette_plane_on_rhand(cgv::render::context& ctx, const rgba& color);
	/// draw a cube on the right hand
	void render_palette_cube_on_rhand(cgv::render::context& ctx, const rgba& color);
	/// draw a cone on the right hand
	void render_palette_cone_on_rhand(cgv::render::context& ctx, const rgba& color);
	/// draw a cylinder on the right hand
	void render_palette_cylinder_on_rhand(cgv::render::context& ctx, const rgba& color);
	
	///
	void clear(cgv::render::context& ctx) override;
	/// 
	bool handle(cgv::gui::event& e) override;
	/// run rollback
	void on_rollback_cb();
	///
	void stream_help(std::ostream& os) override;
	/// compute colors for heights, wnd replace the point cloud colors 
	void colorize_with_height();
	///
	void print_point_cloud_info();

	void create_gui() override;

protected:
	// move reshaped and reordered point cloud data to opengl buffers, does octree generation if required
	// requires source_pc_labels to hold the labels of the points in source_pc, source_pc_labels can be deleted afterwarts
	void prepare_point_cloud() noexcept;

	void on_point_cloud_fit_table();
	//buttons

	void on_load_point_cloud_cb();
	void on_load_palette_labels();
	void on_load_s3d_labels_cb();
	void on_store_s3d_labels_cb();
	void on_generate_large_pc_cb();
	void parallel_saving();
	void on_save_point_cloud_cb();
	void on_clear_point_cloud_cb();
	void on_point_cloud_style_cb();
	void on_lod_mode_change();
	void on_move_to_center();
	// copy marked points to clipboard
	void on_copy_points();
	void on_load_clipboard_point_cloud();
	void on_add_point_cloud_to_clipboard();
	void on_toggle_show_labeled_points_only();

	void on_clear_all_labels();

	//for comparing two point clouds
	void on_load_comparison_point_cloud_1_cb();
	void on_load_comparison_point_cloud_2_cb();
	void on_load_annotated_point_cloud_cb();
	void on_compare_annotated_data();
	void on_load_comparison_point_cloud_cb();
	void on_clear_comparison_point_cloud_cb();
	void on_load_ori_pc_cb();
	void on_load_anno_pcs_cb();
	void getdirectorylist(const std::string& dirpath, std::vector<std::string>& directorylist);
	void getfilelist(const std::string& dirpath, std::vector<std::string>& filelist);
	void on_load_CAD_cb();
	void on_load_scannet_gt_cb();
	///reacts on trigger crossing the threshold
	void on_throttle_threshold(const int ci, const bool low_high);
	/// 
	void on_registration_tool_load_point_cloud();

	void on_auto_pilot_load_path();

	//for time measurement
	void on_set_reduce_cpu_time_watch_file();

	void on_set_reduce_gpu_time_watch_file();

	void on_set_draw_time_watch_file();

	void on_set_labeling_cpu_time_watch_file();

	void on_set_labeling_time_watch_file();

	void on_set_fps_watch_file();

	void on_update_gui();
	/// runs in intervals
	void timer_event(double t, double dt);
	/// enables or disables point cloud fetching from an rgbd device for the rgbd_registration tool
	void fetch_from_rgbd(bool x);
	/// cycle config size preset selection (direction: +1 next, -1 previous)
	void cycle_config_size_preset(int direction);
	/// apply currently selected config size preset to point cloud scale
	void apply_config_size_preset();
	/// return human readable name of selected config size preset
	std::string get_config_size_preset_name() const;
	/// compute absolute point cloud scale for the given config size preset index
	float compute_scale_for_preset(int preset_index) const;

	// --- LABELING_3 control point annotation ---
	void update_cp_params_label(); ///< rebuilds the cp_omega_label text showing all 4 params with selection marker
	/// build ANN kd-tree from source_point_cloud asynchronously for fast KNN queries
	void build_knn_graph_async();
	/// run multi-source Dijkstra from all confirmed control points and apply labels
	void cp_apply_labels();
	
	/// recreates the chunks if the rechunk button is pressed
	void on_rechunk();
	/// disables chunks by merging all points into one chunk
	void on_make_infinite_chunk();
	/// used to change the interaction mode 
	void update_interaction_mode(const InteractionMode im);
	/// updates content of the parameter label that is visible in config mode
	void update_clod_parameters_label();
	/// read palette label names from a file
	void load_label_names_from(const std::string& fn);
	/// read palette label names and colors from a file. 
	/// Lines from the file are expected to look like this:
	/// 1:name=floor
	/// 1:color=ddddFF
	void load_labels_from(const std::string& fn);
	/// load labels from semantic3d ground truth
	void load_label_nr_from(const std::string& fn);
	/// store labels in a file
	void store_label_nr_to(const std::string& fn);
	
	
	std::pair<std::vector<LODPoint>, std::vector<GLuint>> collect_points(cgv::render::context& ctx, int label_groups = (int)point_label_group::SELECTED_BIT);

	// replaces points stored in the clipboard
	void move_points_to_clipboard(std::vector<LODPoint>& points, std::vector<GLuint>& point_ids, GLint* point_labels);
	
	// moves points to a new clipboard slot, sets this slot as the new active slot by replacing palette_clipboard_record_id
	void copy_points_to_clipboard(std::vector<LODPoint>& points, std::vector<GLuint>& point_ids, GLint* point_labels);

	/// push points out of the selection shape
	void push_points(cgv::render::context& ctx, const selection_shape& shape);

	// revert last labeling
	void rollback_last_operation(cgv::render::context& ctx);

	// copies data from opengl buffers to main memory
	void sync_data(int flags= point_attributes::PA_ALL);
	
	// write points from chunks to a point_cloud object
	void copy_chunks_to_point_cloud(point_cloud& dest, std::vector<unsigned>* id_translation_table = nullptr);

	/// hides both help labels and updates li_help. li_help contains label ids for use with text_labels
	void update_help_labels(const InteractionMode mode);

	void update_controller_labels();

	/// update palette color map to reflect changes in PALETTE_COLOR_MAPPING
	void update_palette();
	/// returns true when the controller orientation is in the allowed range to show/use palette
	bool is_palette_activation_pose() const;
	
	vec3 shape_offset(float radius) const;

	/// shows clipboard content in point_cloud_palette
	void show_clipboard_in_palette(pct::point_cloud_clipboard& pcc);

	/// sets/overwrites palette label names acording to the given map, labels not in the map are retained
	/// @param map: id to name mapping
	void set_palette_label_text_map(const std::unordered_map<unsigned int, std::string>& map);

	/// set label colors, labels not in the map stay unchanged
	/// @param map: id to color mapping
	template <typename Color>
	void set_palette_label_color_map(const std::unordered_map<unsigned int, Color>& map) {
		static constexpr int label_offset = 1;
		for (auto& label_color_pair : map) {
			auto ix = label_offset + label_color_pair.first;
			if (ix >= 0 && ix < PALETTE_COLOR_MAPPING.size())
				PALETTE_COLOR_MAPPING[ix] = label_color_pair.second;
		}

		update_palette();
	}
	
	//convert any point format compliant with the octree generator to a lod point
	template <typename T>
	static std::vector<LODPoint> to_lod_points(const std::vector<T>& pnts) {
		std::vector<LODPoint> lod_pnts;
		lod_pnts.reserve(pnts.size());

		for (auto& pnt : pnts) {
			lod_pnts.emplace_back();
			LODPoint& p = lod_pnts.back();
			p.position() = pnt.position();
			p.color() = pnt.color();
			p.level() = pnt.level();
		}
		return lod_pnts;
	}

	/// change pointcloud size
	void rescale_point_cloud(const float new_scale);

	/// teleport to first intersecting point
	bool teleport_ray(const vec3& direction, const vec3& origin, const float ray_radius);

	void swap_point_cloud(pct::point_cloud_record& other);


	/// a quick test that enables to label the point cloud without a VR device 
	void test_labeling_of_points();

	/// relocates points
	void test_moving_points();

	/// load a point cloud and fill all clipboard slots with it
	void test_fill_clipboard();

	std::string get_new_tra_file_name();

	bool save_hmd_tra(const std::string& fn);

	bool load_hmd_tra(const std::string fn);

	void draw_hmd_tra(std::vector<vec3>& hmd_translations, std::vector<vec4>& hmd_colors);

	void on_traject_hmd_cb();
	void on_save_hmd_tra_cb();

	void clear_hmd_tra();

	bool hasEnding(std::string const& str, std::string const& ending);

	void restore_trajectory_file_list(const std::string& dir);

	void read_depth_buffer();

	void draw_surface(cgv::render::context& ctx, bool opaque_part);

	void draw_mesh();

	void adapt_parameters(const int current_framerate);

	vec3 bezier_point(double t, const vec3 p0, vec3 p1, vec3 p2, vec3 p3);

	vec3 interpolate(const vec3& p0, const vec3& p1, float t);

	point_cloud generateBezierSurface(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, int numPoints);

private:
	static constexpr float throttle_threshold = 0.25f; //for on_throttle_threshold

	struct vertex {
		vec3  position;
		float radius;
		rgba  color;
	};
	
	vr_view_interactor* vr_view_ptr;

	pct::point_cloud_server* point_server_ptr;

	pct::waypoint_navigation automated_navigation;
	bool use_autopilot;
	bool navigation_is_controller_path = true;
	int navigation_selected_controller = 1;
	bool start_l;

	point_cloud source_point_cloud;
	std::string source_name;


	bool file_contains_label_groups;

	//for comparing two point clouds, pc_1 is default ground truth
	point_cloud pc_1, pc_2, pc_gt;
	std::vector<point_cloud> pc_list;

	// for loading the annotated ground truth
	point_cloud pc_annotated;

	GLint visible_point_groups;

	size_t max_points;
	float extended_frustum_size; //for multi frame reduction
	float radius_for_test_labeling;

	float teleport_ray_radius; //radius used for the points in the ray intersection test
	vec3 teleport_destination_point;
	float teleport_destination_t;
	bool draw_teleport_destination;
	bool show_teleport_ray;
	double teleport_spere_radius_factor;

	// ── Set-point system (TELEPORT mode) ────────────────────────────────────
	struct SetPoint {
		vec3 local_position; ///< position in point-cloud local space (follows cloud transforms)
		float hit_t;         ///< ray parameter at time of placement (used to scale the marker)
	};
	std::vector<SetPoint> teleport_set_points;   ///< all saved set points
	int teleport_selected_sp = 0;                ///< index of currently selected set point (0-based)
	bool teleport_sp_mode = false;               ///< false = instant teleport, true = set-point placement mode
	uint32_t teleport_sp_label_id = (uint32_t)-1; ///< HUD label on left controller

	
	bool pointcloud_fit_table;
	bool color_based_on_lod;

	pct::point_interaction_settings point_cloud_interaction_settings;
	
	pct::point_cloud_preparation_settings preparation_settings;

	int interaction_mode = (int)InteractionMode::TELEPORT;
	int palette_position = (int)PalettePosition::HEAD_SIDES;
	/// distance of the floating palette panels from the user [m]
	float palette_head_distance = 0.45f;
	/// height of the floating palette panels relative to the head anchor [m]
	float palette_head_height = -0.10f;
	/// angle of each panel away from the forward direction [deg]; left panel is placed at
	/// +angle, the function button panel at -angle
	float palette_panel_angle_deg = 60.0f;
	/// additional drop of the bottom centre toggle panel relative to the side panels [m]
	float palette_toggle_drop = -0.32f;
	/// how much closer to the user the bottom centre toggle panel sits than the side panels [m]
	float palette_toggle_pull = 0.05f;
	/// how strongly each button is tilted around its own horizontal axis to face the headset
	/// centre; 1 aims exactly at the anchor, 0 keeps every panel perfectly vertical
	float palette_button_tilt = 1.0f;
	/// time constant of the yaw follow filter [s]; 0 disables smoothing
	float palette_follow_time = 0.25f;
	/// the panels only start following once the head heading differs by more than this [deg];
	/// inside the dead zone they stay put, beyond it they trail the head by exactly this angle
	float palette_follow_deadband_deg = 60.0f;
	/// visibility of the colour panel, driven by the left toggle button
	bool palette_show_colour_panel = true;
	/// visibility of the function button panel, driven by the right toggle button
	bool palette_show_function_panel = true;
	/// when true the two side panels stay where they are in world space; the bottom toggle
	/// panel keeps following the user so the lock can be released again
	bool palette_lock_side_panels = false;
	/// anchor pose captured at the moment the side panels were locked
	mat34 palette_locked_anchor;
	/// ids and head anchor frame placement of the bottom toggle panel objects (panel 2); these
	/// always follow the user, no matter the palette mode or the lock
	std::vector<int> palette_follow_object_ids;
	std::vector<vec3> palette_follow_object_pos;
	std::vector<quat> palette_follow_object_rot;
	/// last relative transform applied to the following objects, used to skip redundant updates
	mat34 palette_last_follow_rel;
	bool palette_last_follow_rel_valid = false;
	/// yaw-only frame that always follows the user, independent of palette mode and lock
	mat34 head_palette_anchor() const;
	/// frame the main palette panels live in: the controller in hand modes, the head anchor
	/// in the floating mode
	mat34 live_palette_anchor() const;
	/// re-place the bottom panel objects so they keep following the user in every mode
	void update_panel_followers();
	/// apply the panel toggles and the wrist activation gate to the colour and function panels;
	/// the bottom toggle panel is always visible
	void refresh_palette_panel_visibility();
	/// cache of resolved button icon paths, keyed by the file name inside buttons_images/
	std::map<std::string, std::string> button_icon_paths;
	/// click feedback: until what time a button icon should stay darkened
	std::map<int, std::chrono::steady_clock::time_point> palette_icon_flash_until;
	float palette_icon_flash_duration = 0.2f;
	/// locate a file in buttons_images/ and remember the result
	const std::string& resolve_button_icon(const std::string& file_name);
	/// update the palette icons that depend on a toggle state (CP parameter, palette lock and
	/// palette mode buttons)
	void refresh_dynamic_button_icons();
	/// darken button icons according to persistent selections and short click flashes
	void refresh_button_icon_tints();
	/// darken one button icon for palette_icon_flash_duration seconds
	void flash_button_icon(int id);
	/// engage or release the side panel lock, capturing/restoring the frozen anchor
	void apply_palette_lock();
	/// switch the palette between the controller mounted tray and the floating panels
	void set_palette_mode(int position);
	/// split the palette into a colour panel (left) and a function button panel (right)
	bool palette_split_panels = true;
	/// palette object positions as produced by build(), used as the base for every re-layout
	std::vector<vec3> palette_base_positions;
	/// smoothed yaw the floating palette follows [rad]
	float palette_follow_yaw = 0.f;
	bool palette_follow_yaw_valid = false;
	/// smoothed anchor position of the floating palette
	vec3 palette_follow_position = vec3(0.f);
	std::chrono::steady_clock::time_point palette_follow_last_update{};
	/// advance the yaw follow filter; called once per frame from init_frame()
	void update_palette_anchor();
	/// pose used for both rendering and picking the palette; depends on palette_position
	mat34 current_palette_pose() const;
	/// re-applies the palette layout and label anchoring for the active palette_position
	void apply_palette_layout();

	// gui flags

	bool gui_tests = false;
	bool gui_performance = false;
	bool gui_menubar = false;
	bool gui_clod_style = false;
	bool gui_teleport = false;
	bool gui_palette = false;

	bool gui_scaling = false;
	bool gui_marking = false;
	bool gui_rendering = false;
	bool gui_clod_rendering = false;


	//dmat4 model_transform;
	dmat4 concat_mat;
	mat4 hmd_reduction_view_transform; //updated and used in rendering if share_reduction_pass is set

	//cgv::render::clod_point_render_style cp_style;
	cgv::render::cone_render_style cone_style;
	cgv::render::sphere_render_style srs;
	cgv::render::arrow_render_style ars;
	cgv::render::point_render_style point_style;
	cgv::render::cone_render_style rcrs;

	// environment geometry
	pc_cleaning_tool::world world;
	constexpr static float table_height = 0.7f;
	cgv::render::box_render_style box_style;

	std::vector<pct::clod_reduction_state<LODPoint>>  reduction_state;
	int max_reduction_epoch = 0; //how many steps the reduction takes

	bool disable_main_view = false;

	vec3 pos;
	mat3 ori;
	vec3 c_pos;

	vertex p;

	// statistics recording

	/// these queries are polled 
	GLuint gl_render_query[2], reduce_timer_query[2];
	int monitored_eye = 0;
	/// control flags for polling 
	bool poll_query_data, poll_reduce_timer_data;
	GLuint64 diff_draw, diff_reduce_gpu;
	std::chrono::duration<double> diff_reduce_cpu, diff_select;
	stats::scalar_sliding_window<GLuint64> draw_time;  //records last 100 results of diff_draw computations
	stats::scalar_sliding_window<double> reduce_time_cpu;  //records last 100 results of diff_reduce_cpu computations
	stats::scalar_sliding_window<double> reduce_time_gpu;  //records last 100 results of diff_reduce_gpu computations
	size_t sliding_window_size = 100;
	double avg_reduce_time_cpu_view, avg_reduce_time_gpu_view, avg_draw_time_view;
	//store records the time of 1 minute
	std::vector<double> duration_avg_reduce_time_cpu_view, duration_avg_reduce_time_gpu_view, duration_avg_draw_time_view;
	bool enable_performance_stats = false;
	cgv::utils::stopwatch fps_watch;
	std::unique_ptr<std::ofstream> fps_watch_file_ptr, labeling_watch_file_ptr, labeling_cpu_watch_file_ptr, draw_watch_file_ptr, reduce_watch_file_ptr, reduce_cpu_watch_file_ptr;
	std::string fps_watch_file_name, labeling_time_watch_file_name, labeling_cpu_watch_file_name, draw_time_watch_file_name, reduce_time_watch_file_name, reduce_cpu_time_watch_file_name;
	bool log_fps, log_labeling_gpu_time, log_labeling_cpu_time, log_reduce_time, log_reduce_cpu_time, log_draw_time;
	bool adapt_clod;
	int desired_fps = 50;
	int stable_fps = 70;
	float adaptation_rate = 0.05f;
	double frame_rate = 0.0;
	double frame_time = 0.0;

	size_t tool_perf_log_size = 10;
	std::chrono::duration<double> diff_label_cpu;
	stats::scalar_sliding_window<double> labeling_time_cpu;
	stats::scalar_sliding_window<double> labeling_time_gpu;
	double avg_labeling_time_cpu = 0, last_labeling_time_cpu = 0, avg_labeling_time_gpu = 0, last_labeling_time_gpu = 0;

	//config mode
	int config_mode_tool = (int)ConfigModeTools::CMT_CLODParameters;
	// config size presets: 0=original (scale 1), 1=fit 0.5m cube, 2=fit 1.0m cube, 3=fit 2.0m cube
	int config_size_preset_index = 0;

	//spacing tool parameters
	vec3 last_measured_position = vec3(0);
	float last_measured_l0_spacing = 1.0;
	float last_measured_point_spacing = 1.0;
	float last_measured_point_density = 1.0;
	int spacing_tool_label_id = -1;
	int config_help_label_id = -1;
	bool spacing_tool_enabled = false; //set by update_interaction_mode
	std::string spacing_tool_template_str; //for use with snprintf

	int clod_parameters_label_id = -1;
	std::string clod_parameters_template_str;

	float tool_label_scale;
	quat tool_label_ori;
	vec3 right_tool_label_offset, left_tool_label_offset;

	/* stuff regarding point labels */
	
	//shader programs for labeled point rendering with sphere, cuboid and plane selection
	cgv::render::shader_program selection_relabel_prog;
	cgv::render::shader_program label_rollback_prog;
	bool use_label_prog = true;
	bool enable_lights = false;
	cgv::render::surface_render_style point_surface_style;

	
	GLuint lod_to_color_lut_buffer = 0;
	
	static const GLuint lod_color_map_pos = 32;
	
	int label_attribute_id = -1;
	int normal_attribute_id = -1;

	/// reuse filtered points from the first eye for the second eye
	bool share_reduction_pass;
	bool skip_frustum_culling;
	bool chunks_disabled;
	pct::clod_parameter_controller clod_controller;

	// contains the color map
	GLuint color_map_buffer = 0;
	GLuint color_map_buffer_size = 0;
	static const GLuint color_map_layout_pos = 31;
	// stores the old labels and indices of points affected by the last labeling action
	history* history_ptr;

	bool init_label_buffer = true;

	bool gui_cleaning_tools = false;
	// sets the first vr_kits position as pivot point for the clod renderer
	bool use_vr_kit_as_pivot;

	/*gui rendering */
	static constexpr unsigned point_selection_hand = 1; //which hand is used for point selection (0=left, 1=right)
	static constexpr unsigned palette_hand = 0; //which hand is used for point selection (0=left, 1=right)
	static constexpr unsigned mode_selection_hand = 0; //which hand is used for mode selection (0=left, 1=right)
	
	int32_t point_selection_group_mask;
	int32_t point_selection_exclude_group_mask;
	int32_t default_point_selection_group_mask;
	int32_t default_point_selection_exclude_group_mask;
	rgba point_selection_color;
	rgba default_point_selection_color;

	//interacts with labeled points in buffers
	box_selection_tool box_shaped_selection;
	bool box_shaped_selection_is_constraint;
	int selection_touched_corner = -1;

	// control points selection for hole filling
	bool trigger_pressed = false;

	pct::point_cloud_registration_tool point_cloud_registration;
	std::shared_ptr<pct::point_cloud_clipboard> clipboard_ptr;
	pct::clipboard_record_id palette_clipboard_record_id; //referes to a record in clipboard
	int palette_clipboard_point_cloud_id; //referes to an object in the palette
	GLuint clipboard_paste_point_buffer = 0; //buffer for clod renderer
	GLuint clipboard_paste_reduce_ids = 0; //buffer for clod renderer

	//offset for tools on right hand
	vec3 initial_offset_rhand;
	vec3 curr_offset_rhand;

	cgv::render::sphere_render_style sphere_style_rhand;
	cgv::render::sphere_render_style sphere_style_lhand;
	cgv::render::box_render_style cube_style_rhand;
	cgv::render::box_render_style plane_style_rhand;
	
	// cube definition
	float cube_length;
	bool cube_rhand_flat = false;
	box3 cube_rhand;
	quat cube_orientation_rhand;

	//plane definition
	box3 plane_box_rhand;
	/// selection plane orientation (rotate vec3(1,0,0) for the normal)
	quat plane_orientation_rhand;
	bool plane_invert_selection;

	// cone definition (equilateral: height == base_radius)
	float cone_height = 0.04f;
	quat cone_orientation_rhand;
	cgv::render::cone_render_style cone_style_rhand;

	// cylinder definition
	float cylinder_radius = 0.02f;
	float cylinder_height = 0.06f;
	quat cylinder_orientation_rhand;
	cgv::render::cone_render_style cylinder_style_rhand;
	
	
	std::vector<rgba> PALETTE_COLOR_MAPPING; //palette element colors, includes label colors beginning at offset 1 
	vrui::label_palette palette;
	bool palette_pose_gate_enabled = true;
	float palette_yaw_lower_limit_deg = -90.0f;
	float palette_yaw_upper_limit_deg = 45.0f;
	float palette_pitch_lower_limit_deg = -30.0f;
	float palette_pitch_upper_limit_deg = 60.0f;
	float palette_roll_lower_limit_deg = -90.0f;
	float palette_roll_upper_limit_deg = 45.0f;
	vrui::point_cloud_palette rgbd_input_palette;
	/// handle for sphere element used to toggle rgbd_input_palette_delete_mode in the point cloud palette
	int rgbd_input_palette_delete_id;
	bool rgbd_input_palette_delete_mode = false;
	
	//picked_sphere_index means the index of sphere that is rendered on left hand
	int picked_sphere_index;
	int picked_label;
	int picked_semantic_id = 0; ///< raw semantic palette index (without instance encoding)
	point_label_operation picked_label_operation;
	pallete_tool point_editing_tool; //currently active tool in labeling mode
	selection_shape point_selection_shape; //tool shape for point label brush

	// --- Instance labeling ---
	int instance_counter = 1;        ///< current instance id (0 = no instance / unannotated)
	int instance_multiplier = 1000;  ///< multiplier separating instance from semantic (label = semantic*multiplier + instance)
	bool show_instance_colors = false; ///< toggle per-instance color visualization
	/// maps instance_id -> semantic_id to enforce one-semantic-per-instance constraint
	std::unordered_map<int, int> instance_to_semantic;
	uint32_t instance_counter_label_id = (uint32_t)-1; ///< text label handle for palette display
	/// recompute picked_label from instance_counter, picked_semantic_id and current group
	void recompute_instance_label();
	/// check instance-to-semantic constraint, returns true if painting is allowed
	bool check_instance_constraint() const;

	// --- Sculpt-to-label mode (LABELING_2) ---
	bool sculpt_incrop_mode = false;  ///< false = outcrop (hide inside brush), true = incrop (keep inside brush)
	bool sculpt_incrop_triggered = false; ///< edge-detection flag for incrop single-press
	bool sculpt_marker_applied = false;   ///< true once marker bit has been applied to all visible points
	static constexpr GLint SCULPT_MARKER_BIT = 0x0008; ///< bit 3, used to track sculpted points for restore
	uint32_t sculpt_mode_label_id = (uint32_t)-1; ///< text label showing INCROP/OUTCROP state
	uint32_t interaction_mode_label_id = (uint32_t)-1; ///< text label showing current interaction mode on left controller

	// --- Painter mode (LABELING) inpaint/outpaint ---
	bool painter_outpaint_mode = false;  ///< false = inpaint (label inside brush), true = outpaint (label outside brush)
	bool painter_outpaint_triggered = false; ///< edge-detection flag for outpaint single-press
	uint32_t painter_mode_label_id = (uint32_t)-1; ///< text label showing INPAINT/OUTPAINT state

	// --- LABELING_3 control point annotation (Monica et al. 2026) ---
	struct ControlPoint {
		uint32_t point_index; ///< global point ID (index into source_point_cloud)
		GLint label;          ///< label to propagate from this control point
	};
	static constexpr int CP_K = 10;               ///< K neighbors used for KNN graph
	ann_tree cp_ann_tree;                          ///< ANN kd-tree built from source_point_cloud
	std::vector<ControlPoint> cp_control_points;  ///< confirmed control points placed by user
	std::atomic<bool> cp_knn_ready{false};         ///< true when ANN tree has been built
	bool cp_session_active = false;               ///< true once at least one CP has been placed
	bool cp_subtractive_mode = false;             ///< false = add CP mode, true = delete nearest CP mode
	std::future<void> cp_build_future;            ///< handle for background knn build thread
	// ray interaction state
	bool cp_ray_active = false;                   ///< true while right trigger is held
	vec3 cp_hit_point;                            ///< last raycast hit position (world space)
	bool cp_hit_valid = false;                    ///< true if ray currently hits point cloud
	float cp_hit_t = 0.f;                         ///< ray parameter at hit
	// live preview
	std::vector<GLint> cp_preview_labels;         ///< Dijkstra result preview (per-point, -1 = no label)
	void cp_update_preview();                     ///< recompute preview after CP add/delete
	/// request a preview recomputation; the actual (potentially expensive) work is done
	/// throttled on the render thread in init_frame()
	void cp_request_preview_update();
	/// re-tag every control point of the running session with the currently picked label and
	/// request a preview refresh. Must be called whenever picked_label changes (semantic or
	/// instance) so a commit never writes the label that was selected when the session started.
	void cp_sync_session_label();
	/// run the multi source dijkstra propagation over the precomputed knn graph.
	/// results are written to cp_label_result, the visited point ids are collected in cp_touched.
	/// @return number of points that received a label
	size_t cp_propagate_labels();
	/// restore the labels recorded in \p undo_entries (bounds checked) and upload the changed range
	void cp_restore_labels(std::vector<std::pair<uint32_t, GLint>>& undo_entries);
	/// serialize access to cp_ann_tree - the underlying ANN library uses global state and is not thread safe
	std::mutex cp_ann_mutex;
	/// precomputed knn graph, CP_K neighbor ids per point, -1 marks an unused slot.
	/// built once in the background so the propagation does not have to query the kd-tree per node.
	std::vector<int32_t> cp_knn_idx;
	size_t cp_knn_graph_points = 0;               ///< number of points covered by cp_knn_idx
	/// memory budget for the precomputed knn graph; larger clouds fall back to on demand kd-tree queries
	size_t cp_knn_graph_budget_bytes = 768ull * 1024ull * 1024ull;
	// scratch buffers reused between propagation runs to avoid per-call allocation of N-sized vectors
	std::vector<float> cp_dist;
	std::vector<GLint> cp_label_result;
	std::vector<uint32_t> cp_visit_stamp;         ///< generation counter per point, avoids clearing cp_dist every run
	/// generation counter marking points that were already expanded (settled). Dijkstra only ever
	/// has to expand a node once; without this a degenerate edge weight can make the queue cycle.
	std::vector<uint32_t> cp_settled_stamp;
	uint32_t cp_current_stamp = 0;
	std::vector<uint32_t> cp_touched;             ///< point ids reached by the last propagation
	/// hard cap on the number of expanded nodes; protects against unbounded memory use for large Omega
	size_t cp_max_visited_points = 4000000;
	bool cp_preview_dirty = false;                ///< set by interaction code, serviced in init_frame()
	std::chrono::steady_clock::time_point cp_last_preview_time{};
	double cp_preview_min_interval = 0.1;         ///< minimum seconds between two preview recomputations
	// undo support
	std::vector<std::pair<uint32_t, GLint>> cp_undo_buffer; ///< (index, old_label) pairs for the active/preview session

	/// Unified chronological undo stack shared across ALL labeling modes.
	/// Each entry is either a CP-mode operation (cpu_labels non-empty, is_cp=true)
	/// or a history_ptr operation (is_cp=false, one history slot consumed).
	/// Undo always pops the newest entry, giving correct cross-mode LIFO order.
	struct UndoEntry {
		bool is_cp = false;
		std::vector<std::pair<uint32_t, GLint>> cpu_labels; ///< non-empty only when is_cp==true
	};
	std::vector<UndoEntry> unified_undo_stack;
	// tuning parameters
	float cp_Omega_max = 250.0f; ///< maximum path cost for Dijkstra (in units of avg neighbor dist)

	/// Which CP parameter is currently selected for right-stick adjustment.
	/// 0 = Omega_max, 1 = alpha_p, 2 = alpha_n, 3 = alpha_c
	int cp_selected_param = 0;

	float bg_brightness = 1.0f; ///< background brightness: 0=black, 1=white, adjusted in TELEPORT mode in steps of 0.1
	float cp_alpha_p = 1.0f;   ///< position weight in Dijkstra edge cost
	float cp_alpha_n = 1.0f;   ///< normal weight in Dijkstra edge cost
	float cp_alpha_c = 0.5f;   ///< color weight in Dijkstra edge cost
	float cp_avg_spacing = 1.0f; ///< average nearest-neighbor distance (computed at build time)
	uint32_t cp_mode_label_id = (uint32_t)-1; ///< text label showing "ADD CP" / "DEL CP"
	uint32_t cp_omega_label_id = (uint32_t)-1; ///< text label showing current Omega max value

	uint32_t instance_warning_label_id = (uint32_t)-1; ///< pop-up warning shown when instance constraint is violated
	std::chrono::steady_clock::time_point instance_warning_hide_time;

	uint32_t session_warning_label_id = (uint32_t)-1; ///< pop-up warning shown when mode change is blocked by active session
	std::chrono::steady_clock::time_point session_warning_hide_time; ///< when to hide the warning (3s after show)

	selection_shape point_pushing_shape;
	/// store shapes here that must not be used to push points
	std::set<selection_shape> pushing_shape_blacklist;
	std::set<selection_shape> selection_shape_blacklist;
	float radius_adjust_step;
	
	bool gui_chunks = true;
	bool draw_chunk_bounding_boxes = false;

	std::string label_palette_file;

	// IO 
	std::thread* writing_thread;

	// controller positions and orientations
	std::array<mat34,2> controller_poses;
	std::array<float, 8> last_throttle_state;

	bool is_scaling;
	bool is_rotating_moving;
	bool is_tp_floor;
	
	bool paste_pointcloud_follow_controller;
	bool clear_selection_labels_after_copy;

	// intersection points
	pc_cleaning_tool::ray_box_intersection_information intersections;

	bool show_text_labels = true;
	bool show_controller_labels = true;
	bool show_left_bar = false;
	/// stores text labels (text labels means visible text for the user. Not to confuse with point labels)
	vr_labels text_labels;
	/// contains label ids of currently active help text labels
	uint32_t li_help[2];
	bool allways_show_controller_label[2] = { false, false };
	std::array<std::array<uint32_t,2>, NUM_OF_INTERACTIONS> tool_help_label_id;
	std::array<std::array<bool,2>, NUM_OF_INTERACTIONS> tool_help_label_allways_drawn;
	bool always_show_help = false;
	int help_label_ci = 1;

	/// holds controller button labels for both controllers
	std::array<pct::controller_labels,2> controller_labels;
	/// multi dimensional structure that holds handles for use with controller_labels
	std::array<std::array<std::array<std::vector<int>, pct::CLP_NUM_LABEL_PLACEMENTS>, NUM_OF_INTERACTIONS>, 2> controller_label_variants;
	int paste_mode_controller_label_variant_index;
	int box_is_constraint_label_variant;
	int paste_is_shown;
	int clear_box_label_variant;
	int apply_spacing_label_variant;
	rgba controller_label_color;
	int box_is_active_labeling_variant;

	//semantic3d ground truth .labels
	std::vector<rgb> gt_clr;

	std::pair<pointcloud_transformation_state, cgv::render::clod_point_render_style> stored_state;
	bool overview_mode = false;


	std::vector<cgv::pointcloud::point_cloud_provider*> point_cloud_providers;

	pct::bitfield_control_provider<GLint> labeled_point_group_control;
	pct::bitfield_control_provider<GLint> default_point_group_control;
	pct::bitfield_control_provider<GLint> selected_point_group_control;

	//dynamic updated rendering variables 
	std::vector<std::pair<const ivec3, chunk<LODPoint>*>> visible_chunks;

	//debug variables
	std::vector<std::pair<ivec3, chunk<LODPoint>*>> highlighted_chunks;
	std::vector<vec3> highlighted_points;

	//store control points for hole filling
	std::vector<vec3> ctrl_pts;
	std::vector<rgb> ctrl_pts_clrs;
	//std::vector<vec3> surface_pts;
	//std::vector<rgb> surface_pts_clrs;
	point_cloud surface_pts;
	point_cloud filling_pts;
	bool show_surface_pts = false;

	// store trajectory of hmd
	std::vector<vec3> tra_las_points;
	std::vector<vec3> trajectory_points;
	std::vector<quat> tra_las_ori;
	std::vector<quat> trajectory_orientation;
	std::vector<cgv::math::fvec<float, 4>> trajectory_color;
	vec3 hmd_last_pos, hmd_pos;
	quat hmd_last_ori, hmd_ori;
	mat3 hmd_ori3;
	mat34 hmd_pose34, hmd_trans_palette;

	// whether record trajectory of HMD
	bool tra_hmd;
	// whether visualize the trajectory of HMD
	bool is_show_tra;

	/// path to be scanned for drawing files
	std::string draw_file_path;
	/// vector of drawing file names
	std::vector<std::string> draw_file_names;
	/// index of current scene
	int current_drawing_idx;

	//load forklifter mesh
	bool have_new_mesh;
	typedef cgv::media::mesh::simple_mesh<float> mesh_type;
	typedef mesh_type::idx_type idx_type;
	typedef mesh_type::vec3i vec3i;

	bool show_surface;
	cgv::render::CullingMode cull_mode;
	cgv::render::ColorMapping color_mapping;
	rgb  surface_color;
	cgv::render::IlluminationMode illumination_mode;

	bool show_vertices;
	cgv::render::sphere_render_style s_style;
	bool show_wireframe;
	cgv::render::cone_render_style c_style;
	std::string file_name;

	mesh_type M, M2;

	cgv::render::mesh_render_info mesh_info;

	//mode selection
	cgv::render::sphere_render_style sphere_style_lhand_modes;

	std::vector<vec3> mode_spheres_pos;
	std::vector<rgb> mode_spheres_clr;

	bool is_selecting_mode = false;

};

#include <cgv/config/lib_end.h>