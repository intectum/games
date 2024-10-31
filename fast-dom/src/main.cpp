#include <iomanip>
#include <iostream>

#include <ludo/api.h>
#include <ludo/opengl/util.h>

#include "html/attributes.h"
#include "html/layout.h"
#include "html/parsing.h"

auto window_width = uint32_t(1920);
auto window_height = uint32_t(1080);

int main()
{
  // SETUP

  auto inst = ludo::instance();

  auto window = ludo::window { .title = "fast-dom", .width = window_width, .height = window_height, .v_sync = false };
  ludo::init(window);

  auto rendering_context = ludo::rendering_context();
  ludo::init(rendering_context, 0);

  auto camera = ludo::get_camera(rendering_context);
  camera.projection = ludo::orthogonal((float) window_width, (float) window_height, 0.0f, 1.0f);
  ludo::set_camera(rendering_context, camera);

  auto max_instance_count = 100;
  auto rectangle_counts = ludo::rectangle_counts(ludo::vertex_format_pc);

  auto render_commands = ludo::allocate_heap_vram(max_instance_count * sizeof(ludo::render_command));
  auto indices = ludo::allocate_heap_vram(max_instance_count * rectangle_counts.first * sizeof(uint32_t));
  auto vertices = ludo::allocate_heap_vram(max_instance_count * rectangle_counts.second * ludo::vertex_format_p.size);

  auto render_programs = ludo::allocate_array<ludo::render_program>(1);
  auto render_program = ludo::add(render_programs, { .format = ludo::vertex_format_p, .instance_size = 2 * sizeof(ludo::vec3) + sizeof(ludo::vec4) });
  ludo::init(*render_program, ludo::asset_folder + "/shaders/dom.vert", ludo::asset_folder + "/shaders/dom.frag", render_commands, max_instance_count);

  auto rectangle_mesh = ludo::mesh();
  ludo::init(rectangle_mesh, indices, vertices, rectangle_counts.first, rectangle_counts.second, render_program->format.size);
  ludo::rectangle(rectangle_mesh, render_program->format, 0, 0, { .center = ludo::vec3(0.5f, 0.5f, 0.0f) });

  auto rectangle_render_mesh = ludo::render_mesh();
  ludo::init(rectangle_render_mesh);
  ludo::connect(rectangle_render_mesh, *render_program, max_instance_count);
  ludo::connect(rectangle_render_mesh, rectangle_mesh, indices, vertices);

  // PARSE

  auto raw_html = "<body><div style=\"height:50px;background-color:red;\"/><div style=\"width:200px;height:50px;background-color:green;\"/><div style=\"height:50px;background-color:blue;\"/></body>";
  auto html = fast_dom::str { .data = raw_html, .length = (uint32_t) std::strlen(raw_html) };

  auto timer = ludo::timer();

  auto tag_count = fast_dom::count_tags(html);

  auto doc = fast_dom::document();
  doc.names = new fast_dom::str[tag_count];
  doc.attribute_sets = new fast_dom::str[tag_count];
  doc.parent_indices = new uint32_t[tag_count];
  doc.styles = new fast_dom::str*[tag_count];
  for (auto index = uint32_t(0); index < tag_count; index++)
  {
    doc.styles[index] = new fast_dom::str[fast_dom::css_property_count];
  }
  doc.positions = new ludo::vec3[tag_count];
  doc.sizes = new ludo::vec3[tag_count];
  doc.layout_positions = new ludo::vec3[tag_count];
  doc.count = tag_count;

  fast_dom::parse_tags(html, doc);
  fast_dom::parse_attributes(doc);

  std::cout << "parse time: " << std::fixed << std::setprecision(4) << ludo::elapsed(timer) * 1000.0f << "ms" << std::endl;

  // PLAY

  auto frame_count = uint32_t(0);
  auto last_print_time = 0.0f;

  ludo::play(inst, [&](ludo::instance& inst)
  {
    frame_count++;

    ludo::receive_input(window);
    if (window.active_window_frame_button_states[ludo::window_frame_button::CLOSE] == ludo::button_state::UP)
    {
      ludo::stop(inst);
    }

    fast_dom::apply_layout(doc, window_width, window_height);

    auto& positions = doc.positions;
    auto& sizes = doc.sizes;
    auto& styles = doc.styles;

    rectangle_render_mesh.instances.count = 0;
    auto stream = ludo::stream(rectangle_render_mesh.instance_buffer);
    for (auto index = uint32_t(0); index < doc.count; index++)
    {
      auto& position = positions[index];
      auto& size = sizes[index];
      auto& style = styles[index];

      auto background_color = parse_color(style[fast_dom::css_property_background_color]);
      if (background_color == ludo::vec4_zero)
      {
        continue;
      }

      ludo::write(stream, position);
      stream.position += 4; // align 16
      ludo::write(stream, size);
      stream.position += 4; // align 16
      ludo::write(stream, background_color);

      rectangle_render_mesh.instances.count++;
    }

    ludo::start_render_transaction(rendering_context, render_programs);
    ludo::swap_buffers(window);

    ludo::use_and_clear(ludo::frame_buffer { .width = window_width, .height = window_height }, ludo::vec4_one);

    ludo::add_render_command(*render_program, rectangle_render_mesh);

    ludo::commit_render_commands(rendering_context, render_programs, render_commands, indices, vertices);
    ludo::commit_render_transaction(rendering_context);

    if (inst.total_time - last_print_time > 1.0f)
    {
      std::cout << "FPS: " << frame_count << std::endl;
      last_print_time = inst.total_time;
      frame_count = 0;
    }
  });
}
