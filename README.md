# Fbx-Nif-Converter
For converting specific types of Nif &amp; Fbx files back and forth with an optional Vulkan based preview renderer.

Instructions:
Put models in the import directory and run it.
It'll scan the directory for files and spit the converted versions out in export.
Fbx files are always converted to Nif, and Nif files are converted to Fbx.
Running with the exe directly will run with a Vulkan preview renderer if your hardware supports Vulkan and you have up to date drivers.
Running with exportOnly.bat or with the --no-vulkan command line switch will run without the Vulkan preview.


It will report the models as it finds them and provide information about the models inside.
You should be able to read this output to confirm that it is successfully finding the information you want it to carry over in the conversion.
It will print out details about the mesh layout by listing out the vertex attributes that the model uses. This information is useful for identifying which mesh features it successfully imported and identified.
It will also display material information like colors and textures for materials that are configured for use with the models.

If you run into issues please report them to me and I'll work on fixing them asap.
Providing the file that caused the issues along with a description of what went wrong would help a lot too if possible.

Please keep in mind that not all features of the Nif and Fbx formats are supported.
The converter only targets a specific subset of features for specific purposes, but if you have a model with unsupported features and specific use cases in mind I'm willing to take suggestions on what to add next if I have time.
You're also free to modify the source code if you're comfortable with writing C++.
Although please note that the Vulkan graphics engine API is not finalized and may be changed if I update the tool in the future.
I will also be merging the code in this repo into another code base at some point and this code base will no longer be supported, so the form the tool takes may change too if it is updated.

The Vulkan SDK is a requirement to be able to build and run this project for development purposes:

https://www.lunarg.com/vulkan-sdk/

https://vulkan-tutorial.com/Development_environment

Please make sure that the Vulkan headers are on your path and can be included. It's also very helpful if glslangValidator.exe is on your path so that you can conveniently compile shaders.

For people familiar with GPU mesh data:
Nif's vertex attributes are a lot more freeform. They can be configured to have multiple attributes per vertex buffer, and can have varying semantic names for the attributes based on how they're used. The semantic names will be aliased and converted to something the converter knows how to use. They'll be converted back on export.
Fbx's vertex attributes are set up to be one attribute per vertex buffer on the other hand. Fbx doesn't include semantic names or explicit attribute names so the importer will only pick out the ones it knows how to use and use familiar names for them.