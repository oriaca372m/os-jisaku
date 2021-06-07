#include "window.hpp"

Window::Window(int width, int height) : width_{width}, height_{height} {
	data_.resize(height);

	for (int y = 0; y < height; ++y) {
		data_[y].resize(width);
	}
}

void Window::draw_to(PixelWriter& writer, Vector2D<int> position) const {
	if (!transparent_color_) {
		for (int y = 0; y < height(); ++y) {
			for (int x = 0; x < width(); ++x) {
				writer.write(position.x + x, position.y + y, at(x, y));
			}
		}
		return;
	}

	const auto tc = *transparent_color_;
	for (int y = 0; y < height(); ++y) {
		for (int x = 0; x < width(); ++x) {
			const auto& c = at(x, y);
			if (c != tc) {
				writer.write(position.x + x, position.y + y, c);
			}
		}
	}
}

void Window::set_transparent_color(std::optional<PixelColor> c) {
	transparent_color_ = c;
}

Window::WindowWriter* Window::writer() {
	return &writer_;
}

const PixelColor& Window::at(int x, int y) const {
	return data_[y][x];
}

PixelColor& Window::at(int x, int y) {
	return data_[y][x];
}

int Window::width() const {
	return width_;
}

int Window::height() const {
	return height_;
}

Window::WindowWriter::WindowWriter(Window& window) : window_{window} {}

void Window::WindowWriter::write(int x, int y, const PixelColor& c) {
	window_.at(x, y) = c;
}

int Window::WindowWriter::width() const {
	return window_.width();
}
int Window::WindowWriter::height() const {
	return window_.height();
}