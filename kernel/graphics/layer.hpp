#pragma once

#include <memory>

#include "frame_buffer.hpp"
#include "window.hpp"

class Layer {
public:
	Layer(unsigned int id = 0);

	unsigned int id() const;

	Layer& set_window(const std::shared_ptr<Window>& window);
	std::shared_ptr<Window> get_window() const;

	Layer& move(Vector2D<int> pos);
	Layer& move_relative(Vector2D<int> pos_diff);
	const Vector2D<int>& pos() const;

	void draw_to(FrameBuffer& dst) const;

private:
	unsigned int id_;
	Vector2D<int> pos_;
	std::shared_ptr<Window> window_;
};

class LayerManager {
public:
	void set_screen(FrameBuffer* screen);

	Layer& new_layer();

	void draw() const;

	void move(unsigned int id, Vector2D<int> new_position);
	void move_relative(unsigned int id, Vector2D<int> pos_diff);

	void up_down(unsigned int id, int new_height);
	void hide(unsigned int id);

private:
	FrameBuffer* screen_ = nullptr;
	std::vector<std::unique_ptr<Layer>> layers_{};
	std::vector<Layer*> layer_stack_{};
	unsigned int latest_id_ = 0;

	Layer* find_layer(unsigned int id);
};

inline LayerManager* layer_manager = nullptr;
