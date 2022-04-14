#pragma once

#include <memory>
#include <optional>

#include "frame_buffer.hpp"
#include "layer_manager.hpp"

class Layer {
public:
	Layer(LayerManager& manager, unsigned int id = 0);

	unsigned int id() const;
	Vector2D<int> pos() const;
	Vector2D<int> size() const;

	void set_transparent_color(std::optional<PixelColor> c);

	Layer& move(Vector2D<int> pos);
	Layer& move_relative(Vector2D<int> pos_diff);

	void draw_to(FrameBuffer& dst) const;
	void draw_to(FrameBuffer& dst, Rect<int> damage) const;

	// このLayerのコンテンツの範囲rectsが更新された時呼ばれる
	// rectsはこのLayerの座標空間
	void damage(const std::vector<Rect<int>>& rects);

protected:
	LayerManager& manager_;
	std::unique_ptr<FrameBuffer> rendered_;
	std::optional<PixelColor> transparent_color_ = std::nullopt;

	Rect<int> manager_area() const;

private:
	const unsigned int id_;
	Vector2D<int> pos_;
};

class Painter final {
public:
	Painter(FrameBuffer& buffer, Layer& layer);
	~Painter();

	void end();

	void draw_rectangle(const Rect<int>& rect, const PixelColor& c);
	void draw_filled_rectangle(const Rect<int>& rect, const PixelColor& c);
	PixelWriter& pixel_writer();

	PixelWriter& raw_pixel_writer();
	void raw_damage(const Rect<int>& rect);

private:
	FrameBuffer& buffer_;
	Layer& layer_;

	std::vector<Rect<int>> damages_;
};

class BufferLayer final : public Layer {
public:
	BufferLayer(LayerManager& manager, unsigned int id, const FrameBufferConfig& config);

	Painter start_paint();
};

// class LayerManagerLayer final : public Layer {
// 	LayerManagerLayer(LayerManager* parent);
// 	LayerManager canvas;
// };
