#pragma once

#include <Support/WinInclude.h>

#include <cstdlib>
#include <filesystem>
#include <string_view>

class Shader
{
public:
	Shader(std::string_view name);
	~Shader();

private:
	void* m_data = nullptr;
	size_t m_size = 0;
};