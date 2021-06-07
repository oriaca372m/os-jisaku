#pragma once

#include <memory>

#include "window.hpp"

class Layer {
public:
	Layer(unsigned int id = 0);

	unsigned int id() const;

	Layer& set_window(const std::shared_ptr<Window>& window);
	std::shared_ptr<Window> get_window() const;

	Layer& move(Vector2D<int> pos);
	Layer& move_relative(Vector2D<int> pos_diff);

	void draw_to(PixelWriter& writer) const;

private:
	unsigned int id_;
	Vector2D<int> pos_;
	std::shared_ptr<Window> window_;
};

class LayerManager {
public:
	void set_writer(PixelWriter* writer);

	Layer& new_layer();

	void draw() const;

	void move(unsigned int id, Vector2D<int> new_position);
	void move_relative(unsigned int id, Vector2D<int> pos_diff);

	void up_down(unsigned int id, int new_height);
	void hide(unsigned int id);

private:
	PixelWriter* writer_ = nullptr;
	std::vector<std::unique_ptr<Layer>> layers_{};
	std::vector<Layer*> layer_stack_{};
	unsigned int latest_id_ = 0;

	Layer* find_layer(unsigned int id);
};

inline LayerManager* layer_manager;
