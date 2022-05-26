#pragma once

#include "MetaData.h"

namespace Engine
{
	namespace Meta
	{
		struct Scope
		{
			int Indents = 0;

			Scope& operator++()
			{
				++Indents;

				return *this;
			}

			Scope& operator--()
			{
				--Indents;

				return *this;
			}
		};

		void PrintAll(const Engine::Meta::ReflectedTypes* currentScope = Engine::Meta::ReflectedTypes::GetGlobalScope(), Scope scope = Scope());
	}
}