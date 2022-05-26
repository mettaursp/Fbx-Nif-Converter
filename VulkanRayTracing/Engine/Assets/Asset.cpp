#include "Asset.h"

namespace Engine
{
	Asset::~Asset()
	{
		if (Loaded)
			Unload();
	}

	void Asset::SetPath(const std::string& path, AssetType type, OpenMode mode)
	{
		LocalPath = FilePath(path);
		LoadedPath = DirectoryPath;
		LoadedPath += LocalPath;
		Extension = LoadedPath.extension();
		Type = type;
		Mode = mode;
	}

	void Asset::SetImportPath(const std::string& path)
	{
		DirectoryPath = FilePath(path);
	}

	void Asset::SetExportPath(const std::string& path)
	{
		ExportPath = FilePath(path);

		HasSetExportPath = true;
	}

	bool Asset::Load()
	{
		bool canLoad = CanLoad();

		if (canLoad)
		{
			if (Exists())
			{
				std::ifstream file(LoadedPath, std::ios::in | Mode);

				if (!file.is_open())
					return false;

				Loading(file);
			}
			else
			{
				canLoad = LoadDefaultIfNew;

				if (canLoad)
					LoadDefault();
			}
		}

		Loaded |= canLoad;

		return canLoad;
	}

	bool Asset::Unload()
	{
		bool saved = SaveOnUnload && Save();

		Unloading();

		return saved;
	}

	bool Asset::Save()
	{
		bool canSave = Loaded && CanSave();

		if (canSave)
		{
			std::ofstream file(LoadedPath, std::ios::out | Mode);

			if (!file.is_open())
				return false;

			Saving(file, Extension);
		}

		return canSave;
	}

	bool Asset::Export(const std::string& extension, const std::string& newName)
	{
		if (!Loaded) return false;

		FilePath outPath = ExportPath;
		outPath += LocalPath;

		if (!HasSetExportPath)
			outPath = LoadedPath;

		if (newName != "")
			outPath.replace_filename(FilePath(newName));

		FilePath outExtension = FilePath(extension);

		outPath.replace_extension(outExtension);

		std::ofstream file(outPath, std::ios::out | Mode);

		if (!file.is_open())
			return false;

		Saving(file, outExtension);

		return true;
	}

	bool Asset::CanLoad() const
	{
		return LoadDefaultIfNew || Exists();
	}

	bool Asset::CanSave() const
	{
		return !ReadOnly;
	}

	bool Asset::Exists() const
	{
		bool exists = std::filesystem::exists(LoadedPath);
		bool isDirectory = std::filesystem::is_directory(LoadedPath);

		return exists && !isDirectory;
	}
}