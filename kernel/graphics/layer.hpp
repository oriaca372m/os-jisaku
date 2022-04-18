#pragma once

#include <memory>
#include <optional>

#include "frame_buffer.hpp"
#include "layer_manager.hpp"
#include "painter.hpp"

class Layer {
public:
	Layer(LayerManager& manager, unsigned int id);
	virtual ~Layer() = default;

	unsigned int id() const;
	virtual Vector2D<int> pos() const;
	virtual Vector2D<int> size() const = 0;

	void set_transparent_color(std::optional<PixelColor> c);
	bool has_transparency() const;

	Layer& move(Vector2D<int> pos);
	Layer& move_relative(Vector2D<int> pos_diff);

	virtual void draw_to(FrameBuffer& dst) const = 0;
	virtual void draw_to(FrameBuffer& dst, Rect<int> damage) const = 0;

	// このLayerのコンテンツの範囲rectsが更新された時呼ばれる
	// rectsはこのLayerの座標空間
	void damage(const std::vector<Rect<int>>& rects);

	Rect<int> manager_area() const;

protected:
	LayerManager& manager_;
	const unsigned int id_;
	Vector2D<int> pos_ = {0, 0};
	std::optional<PixelColor> transparent_color_ = std::nullopt;
};

class BufferLayer final : public Layer {
public:
	BufferLayer(LayerManager& manager, unsigned int id, PixelFormat pixel_format, Vector2D<int> size);

	Vector2D<int> size() const override;

	void draw_to(FrameBuffer& dst) const override;
	void draw_to(FrameBuffer& dst, Rect<int> damage) const override;

	Painter start_paint();

private:
	std::unique_ptr<FrameBuffer> buffer_;
};

class GroupLayer final : public Layer {
public:
	GroupLayer(LayerManager& manager, unsigned int id, PixelFormat pixel_format, Vector2D<int> size);

	Vector2D<int> size() const override;

	void draw_to(FrameBuffer& dst) const override;
	void draw_to(FrameBuffer& dst, Rect<int> damage) const override;

	LayerManager& layer_manager();

private:
	LayerManager canvas_;
	std::unique_ptr<FrameBuffer> buffer_;
};
