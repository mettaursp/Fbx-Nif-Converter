#pragma once

import <filesystem>;
import <fstream>;

#include <Engine/Objects/Object.h>

namespace Engine
{
	struct AssetTypeEnum
	{
		enum AssetType
		{
			GameAsset,

			Unknown
		};
	};

	class Asset : public Object
	{
	public:
		typedef AssetTypeEnum::AssetType AssetType;
		typedef std::filesystem::path FilePath;
		typedef std::ios_base::openmode OpenMode;

		bool SaveOnUnload = false;
		bool LoadDefaultIfNew = true;
		bool ReadOnly = false;

		~Asset();

		void SetPath(const std::string& path, AssetType type = AssetType::GameAsset, OpenMode mode = OpenMode());
		void SetImportPath(const std::string& path);
		void SetExportPath(const std::string& path);
		bool Load();
		bool Unload();
		bool Save();
		bool Export(const std::string& extension, const std::string& newName = "");
		bool IsLoaded() const { return Loaded; }
		bool CanLoad() const;
		bool CanSave() const;
		bool Exists() const;
		const FilePath& GetLocalPath() const { return LocalPath; }
		const FilePath& GetDirectoryPath() const { return DirectoryPath; }
		const FilePath& GetLoadedPath() const { return LoadedPath; }
		const FilePath& GetExportPath() const { return ExportPath; }
		const FilePath& GetExtension() const { return Extension; }
		AssetType GetAssetType() const { return Type; }
		
		virtual void LoadDefault() {}
		virtual void Loading(std::istream& file) {}
		virtual void Saving(std::ostream& file, const FilePath& extension) {}
		virtual void Unloading() {}

	private:
		FilePath LocalPath;
		FilePath DirectoryPath = FilePath("./assets/");
		FilePath LoadedPath;
		FilePath ExportPath;
		FilePath Extension;
		bool Loaded = false;
		bool HasSetExportPath = false;
		AssetType Type = AssetType::Unknown;
		OpenMode Mode;
	};
}

namespace Enum
{
	typedef Engine::AssetTypeEnum::AssetType AssetType;
}