#pragma once

import <vector>;
import <memory>;
import <fstream>;

#include <Engine/Math/Vector3.h>
#include <Engine/Math/Color4.h>

namespace Engine
{
	namespace Graphics
	{
		struct Vertex
		{
			int Position, Normal, Color, UV;

			Vertex(int position = -1, int uv = -1, int normal = -1, int color = -1) : Position(position), UV(uv), Normal(normal), Color(color) {}
		};

		struct Face
		{
			Vertex Vertices[3];

			std::string Data[3];
		};

		struct ParseModeEnum
		{
			enum Enum
			{
				Seek,
				Position,
				UVs,
				Normal,
				Face,
				Comment,

				Error
			};
		};

		class ObjParser
		{
		public:
			typedef ParseModeEnum::Enum ParseMode;
			typedef std::vector<Face> FaceVector;
			typedef std::vector<Vector3F> VertexVector;
			typedef std::vector<Color4> ColorVector;
			typedef std::vector<char> String;

			FaceVector Faces;
			VertexVector Vertices;
			VertexVector UVs;
			VertexVector Normals;
			ColorVector Colors;

			void Parse(std::istream& stream, const std::string& filePath = "");
			void Parse(const std::string& filePath = std::string(""));

		private:
			ParseMode Mode;
			String Token;
			int TokenNumber;
			Vector3F Vector;
			Face Polygon;

			std::string Output(const std::string& filePath, int lineNumber, const std::string& error);
			void EvaluateToken();
			ParseMode SelectMode(const std::string& token);
			void ReadVertex(const std::string& token, Vertex& vertex);
			bool ReadIndex(const std::string& numberToken, int numberTokenCount, Vertex& vertex);
			bool ReadVector(const std::string& token, int tokenNumber, Vector3F& vector);
			bool ReadVector2(const std::string& token, int tokenNumber, Vector3F& vector);
			bool ReadToken(char character, String& token);
		};
	}
}
