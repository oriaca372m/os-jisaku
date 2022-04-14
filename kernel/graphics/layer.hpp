#pragma once

#include <memory>
#include <optional>

#include "frame_buffer.hpp"
#include "layer_manager.hpp"

class Layer {
public:
	Layer(LayerManager& manager, unsigned int id = 0);
	virtual ~Layer() = default;

	unsigned int id() const;
	virtual Vector2D<int> pos() const;
	virtual Vector2D<int> size() const = 0;

	void set_transparent_color(std::optional<PixelColor> c);

	Layer& move(Vector2D<int> pos);
	Layer& move_relative(Vector2D<int> pos_diff);

	virtual void draw_to(FrameBuffer& dst) const = 0;
	virtual void draw_to(FrameBuffer& dst, Rect<int> damage) const = 0;

	// このLayerのコンテンツの範囲rectsが更新された時呼ばれる
	// rectsはこのLayerの座標空間
	void damage(const std::vector<Rect<int>>& rects);

protected:
	LayerManager& manager_;
	const unsigned int id_;
	Vector2D<int> pos_ = {0, 0};
	std::optional<PixelColor> transparent_color_ = std::nullopt;

	Rect<int> manager_area() const;
};

class Painter final {
public:
	Painter(FrameBuffer& buffer, Layer& layer);
	~Painter();

	void end();

	void copy_y(int dst_y, int src_y, int src_end_y);

	void draw_rectangle(const Rect<int>& rect, const PixelColor& c);
	void draw_filled_rectangle(const Rect<int>& rect, const PixelColor& c);
	PixelWriter& pixel_writer();

	PixelWriter& raw_pixel_writer();
	FrameBuffer& raw_buffer();
	void raw_damage(const Rect<int>& rect);

private:
	FrameBuffer& buffer_;
	Layer& layer_;

	std::vector<Rect<int>> damages_;
};

class BufferLayer final : public Layer {
public:
	BufferLayer(LayerManager& manager, unsigned int id, const FrameBufferConfig& config);

	Vector2D<int> size() const override;

	void draw_to(FrameBuffer& dst) const override;
	void draw_to(FrameBuffer& dst, Rect<int> damage) const override;

	Painter start_paint();

private:
	std::unique_ptr<FrameBuffer> buffer_;
};

// class LayerManagerLayer final : public Layer {
// 	LayerManagerLayer(LayerManager* parent);
// 	LayerManager canvas;
// };
