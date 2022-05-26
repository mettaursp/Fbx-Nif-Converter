#include "FbxWriter.h"

#include "FbxNodes.h"

void FbxWriter::Write(std::ostream& out)
{
	FbxFileStructure fbxFile;

	fbxFile.FbxObjects = FbxObjects;
	fbxFile.FbxMaterials = FbxMaterials;
	fbxFile.Package = Package;

	fbxFile.MakeFileStructure();
	fbxFile.FinalizeNodes();
	fbxFile.WriteNodes(out);
}