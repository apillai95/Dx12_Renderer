#include "Shader.h"

Shader::Shader(std::string_view name)
{
	wchar_t moduleFileName[MAX_PATH];
	GetModuleFileNameW(nullptr, moduleFileName, MAX_PATH);
}

Shader::~Shader()
{
	if (m_data)
		free(m_data);
}
