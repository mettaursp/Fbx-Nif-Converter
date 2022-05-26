#include "ObjParser.h"
import <cmath>;

import <fstream>;
import <sstream>;
import <exception>;

namespace Engine
{
	namespace Graphics
	{
		std::string ObjParser::Output(const std::string& filePath, int lineNumber, const std::string& error)
		{
			std::stringstream out;

			out << filePath << " [ " << lineNumber << " ]: " << error;

			return out.str();
		}

		Vector3F cross(const Vector3F& vert1, const Vector3F& vert2, const Vector3F& vert3)
		{
			Vector3F vec1(vert2.X - vert1.X, vert2.Y - vert1.Y, vert2.Z - vert1.Z);
			Vector3F vec2(vert3.X - vert1.X, vert3.Y - vert1.Y, vert3.Z - vert1.Z);
			Vector3F normal(
				vec1.Y * vec2.Z - vec1.Z * vec2.Y,
				vec1.Z * vec2.X - vec1.X * vec2.Z,
				vec1.X * vec2.Y - vec1.Y * vec2.X
			);

			float length = 1 / sqrt(normal.X * normal.X + normal.Y * normal.Y + normal.Z * normal.Z);

			normal.X *= length;
			normal.Y *= length;
			normal.Z *= length;

			return normal;
		}

		void ObjParser::Parse(std::istream& file, const std::string& filePath)
		{
			unsigned int col = Color4(0.5f, 1, 0.25f, 1);
			Color4 rgb = Color4(col);

			Mode = ParseMode::Seek;

			Token.clear();
			TokenNumber = 0;
			int line = 0;

			Faces.clear();
			Vertices.clear();
			UVs.clear();
			Normals.clear();

			Vector.Set(0, 0, 0);

			try
			{
				while (!file.eof())
				{
					char character;

					file >> std::noskipws >> character;

					std::string numberToken = "";

					bool finished = ReadToken(character, Token);

					if (finished)
						EvaluateToken();

					if (character == '\n')
					{
						Mode = ParseMode::Seek;
						TokenNumber = 0;
						++line;
					}
				}

				if (Token.size() > 0)
					EvaluateToken();

				for (int index = 0; index < int(Faces.size()); ++index)
				{
					auto i = &Faces[index];

					Vector3F normal = (Vertices[i->Vertices[1].Position] - Vertices[i->Vertices[0].Position]).Cross(Vertices[i->Vertices[2].Position] - Vertices[i->Vertices[0].Position]);

					float length = normal.SquareLength();

					if (length < 1e-15)
					{
						std::swap(Faces[index], Faces.back());
						Faces.pop_back();
						--index;

						continue;
					}

					normal *= 1 / std::sqrt(length);

					for (int j = 0; j < 3; ++j)
					{
						Vertex& vertex = i->Vertices[j];

						if (vertex.Normal == -1)
						{
							vertex.Normal = int(Normals.size());
							Normals.push_back(normal);
						}

						if (vertex.Color == -1)
							vertex.Color = int(Colors.size());

						if (vertex.UV == -1)
							vertex.UV = int(UVs.size());
					}
				}

				Colors.push_back(Color4(1.f, 1.f, 1.f, 1.f));
				UVs.push_back(Vector3F());
			}
			catch (std::string& error)
			{
				throw Output(filePath, line, error);
			}
		}

		void ObjParser::Parse(const std::string& filePath)
		{
			std::ifstream file(filePath, std::ios_base::in);

			if (!file.is_open() || !file.good())
				throw std::string(std::system_error(errno, std::system_category(), "failed to open " + filePath).what());

			Parse(file, filePath);
		}

		void ObjParser::EvaluateToken()
		{
			bool finished = false;

			Token.push_back(0);
			std::string tokenString(Token.data());

			switch (Mode)
			{
			case ParseMode::Seek:
				Mode = SelectMode(tokenString);
				Vector.Set(0, 0, 0, 0);
				Token.clear();

				break;
			case ParseMode::Position:
				finished = ReadVector(tokenString, TokenNumber - 1, Vector);

				if (finished)
				{
					Vector.W = 1;

					Vertices.push_back(Vector);
				}

				break;
			case ParseMode::UVs:
				finished = ReadVector2(tokenString, TokenNumber - 1, Vector);

				if (finished)
					UVs.push_back(Vector);

				break;
			case ParseMode::Normal:
				finished = ReadVector(tokenString, TokenNumber - 1, Vector);

				if (finished)
					Normals.push_back(Vector);

				break;
			case ParseMode::Face:
				if (TokenNumber > 3)
					throw std::string("model isn't triangulated");

				ReadVertex(tokenString, Polygon.Vertices[TokenNumber - 1]);

				Polygon.Data[TokenNumber - 1] = tokenString;

				if (TokenNumber == 3)
					Faces.push_back(Polygon);

				break;
			case ParseMode::Error:

				throw std::string("error loading test_model.Obj");
			}

			Token.clear();
			++TokenNumber;
		}

		ObjParser::ParseMode ObjParser::SelectMode(const std::string& token)
		{
			if (token == "#" || token == "mtllib" || token == "usemtl" || token == "o" || token == "g" || token == "s")
				return ParseMode::Comment;
			else if (token == "v")
				return ParseMode::Position;
			else if (token == "vt")
				return ParseMode::UVs;
			else if (token == "vn")
				return ParseMode::Normal;
			else if (token == "f")
				return ParseMode::Face;
			else
				throw std::string("unsupported data");
		}

		void ObjParser::ReadVertex(const std::string& token, Vertex& vertex)
		{
			std::string numberToken;

			int numberTokenCount = 0;

			vertex.Position = -1;
			vertex.UV = -1;
			vertex.Normal = -1;

			for (int i = 0; i < int(token.length()); ++i)
			{
				char vertexCharacter = token[i];

				if (vertexCharacter == '/')
				{
					ReadIndex(numberToken, numberTokenCount, vertex);

					numberToken = "";
					++numberTokenCount;
				}
				else
					numberToken += vertexCharacter;
			}

			ReadIndex(numberToken, numberTokenCount, vertex);
			++numberTokenCount;
		}

		bool ObjParser::ReadIndex(const std::string& numberToken, int numberTokenCount, Vertex& vertex)
		{
			if (numberToken != "")
			{
				int number = int(atoi(numberToken.c_str())) - 1;

				switch (numberTokenCount)
				{
				case 0:
					vertex.Position = number;

					break;
				case 1:
					vertex.UV = number;

					break;
				case 2:
					vertex.Normal = number;

					return true;
				default:
					throw std::string("too many vertex parameters");
				}
			}

			return false;
		}

		bool ObjParser::ReadVector2(const std::string& token, int tokenNumber, Vector3F& vector)
		{
			float number = float(atof(token.c_str()));

			switch (tokenNumber)
			{
			case 0:
				vector.X = number;

				break;
			case 1:
				vector.Y = number;

				return true;
			default:
				throw std::string("too many vector2 parameters");
			}

			return false;
		}

		bool ObjParser::ReadVector(const std::string& token, int tokenNumber, Vector3F& vector)
		{
			float number = float(atof(token.c_str()));

			switch (tokenNumber)
			{
			case 0:
				vector.X = number;

				break;
			case 1:
				vector.Y = number;

				break;
			case 2:
				vector.Z = number;

				return true;
			default:
				throw std::string("too many vector parameters");
			}

			return false;
		}

		bool ObjParser::ReadToken(char character, String& token)
		{
			if (character == ' ' || character == '\n')
				return token.size() != 0;

			token.push_back(character);

			if (token.size() == 1 && token.at(0) == '#')
				return true;

			return false;
		}
	}
}