#pragma once
#include <vector>
#include <functional>
#include <cgv/render/drawable.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/box_wire_renderer.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv_gl/rectangle_renderer.h>
#include <memory>
#include "vr_labels.h"
#include "label_shader_manager.h"
#include <cgv/render/shader_program.h>
#include <cgv/render/texture.h>
#include <cgv_gl/point_renderer.h>
#include <point_cloud.h>

namespace vrui {

	// used to control palette rendering
	enum PaletteObject {
		PO_NONE = 0,
		PO_SPHERE = 1,
		PO_PLANE = 2,
		PO_CUBOID = 3,
		PO_BOX_FRAME = 4,
		PO_SPHERE_FRAME = 5,
		PO_BOX_PLANE = 6,
		PO_CONE = 7,
		PO_CYLINDER = 8,
		NUM_PALETTE_OBJECTS
	};

	enum PaletteObjectGroup {
		POG_NONE = 0,
		POG_TOP_TOOLBAR = 1,
		POG_LEFT_TOOLBAR = 2
	};

	class palette;

	/// represents/refers to an object picked from a palette
	class picked_object {
		palette* palette_ptr;
		int id_p;
	public:
		picked_object(palette& p, int id);
		///returs pointer set by palette::set_object_data(..)
		void* data();
		int position_in_group();
		int id();
		PaletteObjectGroup object_group();
		PaletteObject object_type();
		/// @return pointer to palette containing the object refered by this picked_object instance
		palette* get_palette_ptr();
	};

	class palette {
	public:
		constexpr static float icosahedron_scale = 0.020f;
	private:
		std::vector<PaletteObject> palette_object_shapes; //geometric shapes of palette objects
		std::vector<PaletteObjectGroup> palette_object_groups;
		std::vector<bool> palette_object_visibility;
		/// panel each object belongs to; panels can be shown/hidden as a whole
		std::vector<int> palette_object_panel;
		static constexpr int max_panels = 4;
		bool palette_panel_visible[max_panels] = { true, true, true, true };
		std::vector<cgv::vec3> palette_object_positions;
		/// extra rotation per object, applied on top of the shape specific base orientation.
		/// lets objects be arranged on panels that are turned relative to the palette pose.
		std::vector<cgv::quat> palette_object_rotations;
		std::vector<cgv::rgba> palette_object_colors;
		std::vector<void*> palette_object_data;
		std::vector<int> positions_in_group;
		std::vector<bool> palette_object_use_button_image;
		std::vector<std::string> palette_object_button_image_files;
		std::vector<cgv::render::texture> palette_object_button_textures;
		std::vector<bool> palette_object_button_texture_needs_upload;
		std::vector<cgv::rgba> palette_object_icon_tints;
		std::vector<int> palette_text_label_ids; //map containing text label ids for text attached to palette objects, -1 means no label is assigned
		vr_labels palette_text_labels;
		// How the text labels of palette objects are anchored. This has to match the pose handed
		// to render_palette(): a palette floating in front of the head needs its labels in the head
		// coordinate system, one held by a controller needs them on that controller.
		CoordinateSystem label_coord_system = CS_LEFT_CONTROLLER;
		/// base orientation that lays a label flat into the palettes local xz plane
		cgv::quat label_base_orientation = cgv::quat(0.7071068f, -0.7071068f, 0.f, 0.f);

		std::function<void(picked_object)> palette_picking_function;
		int last_triggered_object;

		//groups
		std::vector<int> top_toolbar_object_group;
		std::vector<int> left_toolbar_object_group;
		bool top_toolbar_visible;
		bool palette_changed = false;
		//renderer
		cgv::render::box_renderer palette_box_renderer;
		cgv::render::sphere_renderer palette_sphere_renderer;
		cgv::render::box_renderer palette_plane_renderer;
		cgv::render::box_wire_renderer palette_box_wire_renderer;
		cgv::render::box_renderer palette_box_plane_renderer;  // render rounding corner box renderer for palette
		cgv::render::rectangle_renderer palette_icon_renderer;
		cgv::render::attribute_array_manager palette_spheres;
		cgv::render::attribute_array_manager palette_boxes;
		cgv::render::attribute_array_manager palette_planes;
		cgv::render::attribute_array_manager palette_box_frames;
		cgv::render::attribute_array_manager palette_box_planes; // render rounding corner box renderer for palette
		std::vector<std::vector<unsigned int>> palette_indices; //used for indexed rendering
		cgv::render::sphere_render_style p_sphere_style;
		cgv::render::box_render_style p_box_style, p_plane_style, p_box_plane_style; // p_box_plane_style: render rounding corner box renderer for palette
		cgv::render::box_wire_render_style p_box_wire_style;
		cgv::render::rectangle_render_style p_icon_style;
		cgv::render::texture palette_button_texture;
		std::string palette_button_image_file;
		bool palette_button_texture_needs_upload = false;
	protected:
		cgv::render::point_render_style point_cloud_style;
	private:
		GLuint sphere_frame_point_buffer;
		GLuint sphere_frame_vao;
		GLuint sphere_frame_offset_buffer;
		GLuint sphere_frame_color_buffer;

		std::vector<cgv::vec3> sphere_points; 		/// stores points of a geodesic sphere
		std::vector<unsigned> sphere_triangles; /// stores indices into sphere points defining triangle of a geodesic sphere

		// scratch buffers reused by render_palette every frame; kept as members so the per frame
		// icon/cone/cylinder rendering does not allocate and free a handful of vectors per eye
		struct icon_draw_entry {
			cgv::render::texture* tex;
			cgv::vec3 position;
			cgv::quat rotation;
			cgv::rgba tint;
		};
		std::vector<icon_draw_entry> icon_draw_list;
		std::vector<cgv::vec3> icon_batch_positions;
		std::vector<cgv::quat> icon_batch_rotations;
		std::vector<cgv::rgba> icon_batch_colors;
		std::vector<cgv::math::fvec<float, 2>> icon_batch_extents;
		std::vector<cgv::math::fvec<float, 4>> icon_batch_texcoords;
		std::vector<cgv::vec3> cone_scratch_positions;
		std::vector<float> cone_scratch_radii;
		std::vector<cgv::rgba> cone_scratch_colors;

		std::vector<cgv::vec3> point_cloud_positions;
		std::vector<cgv::quat> point_cloud_rotations;
		std::vector<float> point_cloud_scales;

		std::vector<std::vector<cgv::vec3>> point_clouds_point_positions;
		std::vector<std::vector<cgv::rgb8>> point_clouds_point_colors;
		std::vector<cgv::render::attribute_array_manager> point_cloud_array_managers;

		cgv::render::context* ctx_ptr;

	protected:
		palette();

		virtual ~palette(); // virtual destructor so pointers to palettes can be dynamic casted (picked_object uses one)
		/// @return number of objects in the palette
		size_t num_objects() const;
		/// (re)place the text label of \p obj_ix from its current position and the current label anchor
		void place_object_label(unsigned int obj_ix);

		cgv::render::shader_program& instanced_rendering_prog();
	public:
		using object_funct = std::function<void(picked_object)>;
		/** add a new object to the palette
		*	@param[in] shape : shape of the object
		*	@param[in] color : color of the object
		*	@param[in] pog : grouping of the object
		*	@returns handle for the object
		*/
		int add_object(const PaletteObject shape, const cgv::vec3& position, const cgv::rgba& color, const PaletteObjectGroup pog=POG_NONE);
		/** add a pointcloud that is rendered inside the palette
		*	@param[in] point_positions : pointer to first element in an array/vector of point positions
		*	@param[in] point_colors : pointer to first element in an array/vector of point colors
		*	@param[in] position : position/offset of the pointcloud relative to the palettes coordinate system
		*	@param[in] rotation : rotation of the pointcloud relative to the palettes coordinate system
		*	@returns handle for the object
		*/
		int add_pointcloud(const cgv::vec3* point_positions, const cgv::rgb8* point_colors, const size_t size, const cgv::vec3& position, const cgv::quat& rotation);

		void replace_pointcloud(const int pc_id, const cgv::vec3* point_positions, const cgv::rgb8* point_colors, const size_t size);
		
		void replace_pointcloud(const int pc_id, const cgv::vec3* point_positions, const cgv::rgb8* point_colors, const size_t size, const cgv::vec3& position, const cgv::quat& rotation);

		/** associate a function with an object, this functor can be called by trigger_function(id)
			@param id : object id
			@param funct : function/callable wrapped into std::function<void(picked_object)>
		*/
		virtual void set_function(object_funct& func);
		// call the function set by set_function() with parameters generated from the given id
		void trigger_function(int id);
		/// reset one-shot trigger state; useful when palette interaction is temporarily disabled
		void reset_trigger_state() { last_triggered_object = -1; }
		// return a vector containing all object ids used in the left toolbar 
		const std::vector<int>& left_toolbar();

		//tests if pnt is inside any palette objects radius, 
		//@param[in] pnt: point to check
		//@param[in] palette_pose
		//@param[out] dist: distance to picked object's center
		//@returns id if successful and -1 if no object was picked
		int pick_object(const cgv::vec3& pnt, const cgv::mat34& palette_pose, float& dist);

		int trigger_object(const cgv::vec3& pnt, const cgv::mat34& palette_pose, float& dist, bool only_once_between_objects = true);

		/** set / changes text of the textlable linked to the object refered by \p obj_ix
		*	@param obj_ix : object id
		*	@param txt : text to put on the label
		*/
		void set_label_text(unsigned int obj_ix, std::string txt);
		
		const std::string& get_label_text(unsigned obj_ix);

		/// local position of a palette object; returns a zero vector for invalid ids
		const cgv::vec3& object_position(const int id) const;
		/// move a palette object inside the palettes local coordinate system and re-place its text label
		void set_object_position(const int id, const cgv::vec3& position);
		/// move and rotate a palette object; \p rotation is applied on top of the shape base orientation
		void set_object_placement(const int id, const cgv::vec3& position, const cgv::quat& rotation);
		/// assign an object to a panel so it can be shown/hidden together with the rest of that panel
		void set_object_panel(const int id, const int panel);
		/// show/hide a whole panel; combined with the per object visibility
		void set_panel_visible(const int panel, const bool visible);
		/// @return visibility of a whole panel
		bool is_panel_visible(const int panel) const;
		/// @return true if the object is visible on its own and its panel is shown
		bool object_is_visible(const int id) const;
		/// set the coordinate system the palette text labels are placed in; must match the frame
		/// the pose handed to render_palette() is expressed in
		void set_label_coordinate_system(CoordinateSystem coord_system);
		/// access to the label renderer, e.g. to drive CS_CUSTOM
		vr_labels& ref_text_labels() { return palette_text_labels; }
		/// re-place every existing text label, e.g. after objects moved or the label anchor changed
		void refresh_label_placement();

		// set a pointer that is passed to the objects functor if invoked by trigger_function(..)
		void set_object_data(const int id, void* data_ptr);
		// renders the palette, call this inside your draw loop
		void render_palette(cgv::render::context& ctx, const cgv::mat34& pose);

		void set_colors(const std::vector<cgv::rgba>& palette_colors);
		/// change visibility of the objects added over add_object(..) with pog=PaletteObjectGroup::POG_TOP_TOOLBAR 
		void set_top_toolbar_visibility(const bool visibility);
		/// set visibility of a single palette object by global id; safe to call with an invalid id
		void set_object_visibility(int id, bool visible) {
			if (id >= 0 && id < (int)palette_object_visibility.size()) {
				palette_object_visibility[id] = visible;
				refresh_object_label_visibility(id);
				set_palette_changed();
			}
		}
		/// show or hide the text label of an object according to its own and its panels visibility.
		/// Every path that changes label text or visibility has to go through this, otherwise a
		/// label can outlive the object it belongs to (e.g. re-labelling a button on a hidden panel).
		void refresh_object_label_visibility(int id) {
			if (id < 0 || id >= (int)palette_text_label_ids.size())
				return;
			const int li = palette_text_label_ids[id];
			if (li < 0)
				return;
			if (object_is_visible(id))
				palette_text_labels.show_label(li);
			else
				palette_text_labels.hide_label(li);
		}
		/// hide the text label associated with a palette object (safe to call with invalid id or before label is created)
		void hide_object_label(int id) {
			if (id >= 0 && id < (int)palette_text_label_ids.size()) {
				int li = palette_text_label_ids[id];
				if (li >= 0)
					palette_text_labels.hide_label(li);
			}
		}
		/// show the text label of a palette object, unless the object or its panel is hidden
		void show_object_label(int id) {
			refresh_object_label_visibility(id);
		}
		/// indicate that the palette was changed so class internal render code can react to it
		void set_palette_changed();

		/// set one common image file used for all buttons flagged with set_object_uses_button_image()
		void set_buttons_image_file(const std::string& file_name);
		/// enable/disable button-image rendering for a specific palette object id; safe to call with invalid ids
		void set_object_uses_button_image(int id, bool use_image = true);
		/// set an image file for one palette object; if empty, fallback image from set_buttons_image_file() is used
		void set_object_button_image_file(int id, const std::string& file_name);
		/// tint multiplier for one button icon (1,1,1,1 keeps the original icon colors)
		void set_object_icon_tint(int id, const cgv::rgba& tint);

		void init_text_labels(cgv::render::context& ctx, vr_view_interactor* vr_view_ptr);

		void init_frame(cgv::render::context& ctx);
		/// initializes/creates opengl buffers
		void init(cgv::render::context& ctx);
		/// cleanup
		void clear(cgv::render::context& ctx);
		/// check if a object id/handle is valid
		bool object_id_is_valid(const int id) const;
		/// check if a pointcloud id/handle is valid
		bool pointcloud_id_is_valid(const int id) const;

		cgv::rgba& object_color(const int id);
		/// access to the palette objects' render styles
		cgv::render::point_render_style& point_style();
		/// access to the palette objects' render styles
		cgv::render::sphere_render_style& sphere_style();
		/// access to the palette objects' render styles
		cgv::render::box_render_style& box_style();
		/// access to the palette objects' render styles
		cgv::render::box_render_style& plane_style();
		/// access to the palette objects' render styles
		cgv::render::box_wire_render_style& wire_box_style();
		/// access to the palette objects' render styles
		cgv::render::box_render_style& box_plane_style();

		/** \brief generates points for a pointcloud preview with fixed point density
		  * \param[in] points            a pointcloud
		  *	\param[out] point_positions	 preview point positions
		  * \param[out] point_colors     preview point colors */
		static void generate_preview(point_cloud& points, std::vector<cgv::vec3>& point_positions, std::vector<cgv::rgb8>& point_colors, int detail_level = 0);


		friend class picked_object;
	};

}

