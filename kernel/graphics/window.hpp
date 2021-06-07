#pragma once

#include <optional>
#include <vector>

#include "graphics.hpp"

class Window final {
public:
	class WindowWriter : public PixelWriter {
	public:
		explicit WindowWriter(Window& window);
		void write(int x, int y, const PixelColor& c) override;

		int width() const;
		int height() const;

	private:
		Window& window_;
	};

	Window(int width, int height);
	~Window() = default;

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	void draw_to(PixelWriter& writer, Vector2D<int> position) const;

	void set_transparent_color(std::optional<PixelColor> c);

	WindowWriter* writer();

	const PixelColor& at(int x, int y) const;
	PixelColor& at(int x, int y);

	int width() const;
	int height() const;

private:
	int width_;
	int height_;

	std::vector<std::vector<PixelColor>> data_{};
	WindowWriter writer_{*this};
	std::optional<PixelColor> transparent_color_ = std::nullopt;
};
