#include "DebugPrintMeta.h"

import <iostream>;

namespace Engine
{
	namespace Meta
	{
		std::ostream& operator<<(std::ostream& out, const Scope& scope)
		{
			for (int i = 0; i < scope.Indents; ++i)
				out << "\t";

			return out;
		}

		void Print(const Engine::Meta::Function& member, Scope scope)
		{
			if (member.IsConstructor)
				std::cout << scope << "\tconstructor " << member.Name << " {" << std::endl;
			else
				std::cout << scope << "\tfunction " << member.Name << " from " << member.Parent->Name << " {" << std::endl;

			for (int i = 0; i < int(member.Overloads.size()); ++i)
			{
				auto& overload = member.Overloads[i];

				std::cout << scope << "\t\t";

				if (overload.IsStatic)
					std::cout << "static ";

				std::cout << overload.ReturnType->Name << " " << member.Name << "(";

				for (int i = 0; i < int(overload.Parameters.size()); ++i)
				{
					auto& parameter = overload.Parameters[i];

					if (i > 0)
						std::cout << ",";

					std::cout << " " << parameter.Type->Name << " " << parameter.Name;

					if (parameter.DefaultValue)
						std::cout << " = " << parameter.DefaultValue;
				}

				std::cout << " )";

				if (overload.IsConst)
					std::cout << " const";

				std::cout << std::endl;
			}

			std::cout << scope << "\t}" << std::endl;
		}

		void Print(const Engine::Meta::Event& member, Scope scope)
		{
			std::cout << scope << "\t";

			if (member.IsStatic)
				std::cout << "static ";

			std::cout << "event " << member.Name << " <";

			for (int i = 0; i < int(member.Parameters.size()); ++i)
			{
				auto& parameter = member.Parameters[i];

				if (i > 0)
					std::cout << ",";

				std::cout << " " << parameter.Type->Name << " " << parameter.Name;
			}

			std::cout << " > from " << member.Parent->Name << std::endl;
		}

		void PrintAll(const Engine::Meta::ReflectedTypes* currentScope, Scope scope)
		{
			if (currentScope == nullptr) return;

			std::cout << scope << "namespace " << currentScope->GetName() << " {" << std::endl;

			++scope;

			for (int i = 0; i < currentScope->GetCount(); ++i)
			{
				auto type = currentScope->Get(i);

				std::cout << scope << type->Name;

				if (type->Parent)
					std::cout << " : " << type->Parent->Name;

				if (!type->IsFundamental || type->IsEnum)
				{
					std::cout << std::endl << scope << "{" << std::endl;

					if (!type->IsEnum)
						Print(type->Constructor, scope);

					for (int i = 0; i < int(type->Members.size()); ++i)
					{
						std::cout << scope << "\t";

						auto& member = type->Members[i];

						if (member.IsStatic)
							std::cout << "static ";

						if (member.IsConst)
							std::cout << "const ";

						std::cout << member.Type->Name << " " << member.Name << " from: " << member.Parent->Name << std::endl;
					}

					for (int i = 0; i < int(type->Events.size()); ++i)
						Print(type->Events[i], scope);

					for (int i = 0; i < int(type->Properties.size()); ++i)
					{
						std::cout << scope << "\t";

						auto& member = type->Properties[i];

						if (member.IsStatic)
							std::cout << "static ";

						std::cout << "property " << member.Name << " from " << member.Parent->Name << " {" << std::endl;

						std::cout << scope << "\t\tGetter<" << member.Getter.Type->Name << "> " << std::endl;

						for (int i = 0; i < int(member.Setters.size()); ++i)
							std::cout << scope << "\t\tSetter<" << member.Setters[i].Type->Name << "> " << std::endl;

						std::cout << scope << "\t}" << std::endl;
					}

					for (int i = 0; i < int(type->Functions.size()); ++i)
						Print(type->Functions[i], scope);

					for (const EnumItem& item : type->EnumItems)
						std::cout << scope << "\titem " << item.Name << "[" << item.Value << "]" << std::endl;

					for (const auto& item : type->UnorderedEnumItems)
						std::cout << scope << "\titem " << item.second.Name << "[" << item.second.Value << "]" << std::endl;

					std::cout << scope << "}" << std::endl;
				}

				std::cout << scope << std::endl;
			}

			for (int i = 0; i < currentScope->GetScopeCount(); ++i)
				PrintAll(currentScope->GetScope(i), scope);

			--scope;

			std::cout << scope << "}" << std::endl;
		}
	}
}