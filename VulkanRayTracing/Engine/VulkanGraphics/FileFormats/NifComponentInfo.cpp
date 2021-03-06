#include "NifComponentInfo.h"


std::map<ComponentFormat, ComponentInformation> ComponentInfo = {
	{
		ComponentFormat::F_UNKNOWN,
		ComponentInformation{
			Enum::AttributeDataType::Unknown,
			0
		}
	},
	{
		ComponentFormat::F_INT8_1,
		ComponentInformation{
			Enum::AttributeDataType::Int8,
			1
		}
	},
	{
		ComponentFormat::F_UINT8_1,
		ComponentInformation{
			Enum::AttributeDataType::UInt8,
			1
		}
	},
	{
		ComponentFormat::F_NORMINT8_1,
		ComponentInformation{
			Enum::AttributeDataType::UInt8,
			1
		}
	},
	{
		ComponentFormat::F_NORMUINT8_1,
		ComponentInformation{
			Enum::AttributeDataType::UInt8,
			1
		}
	},
	{
		ComponentFormat::F_INT16_1,
		ComponentInformation{
			Enum::AttributeDataType::Int16,
			1
		}
	},
	{
		ComponentFormat::F_UINT16_1,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			1
		}
	},
	{
		ComponentFormat::F_NORMINT16_1,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			1
		}
	},
	{
		ComponentFormat::F_NORMUINT16_1,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			1
		}
	},
	{
		ComponentFormat::F_FLOAT16_1,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			1
		}
	},
	{
		ComponentFormat::F_INT32_1,
		ComponentInformation{
			Enum::AttributeDataType::Int32,
			1
		}
	},
	{
		ComponentFormat::F_UINT32_1,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			1
		}
	},
	{
		ComponentFormat::F_NORMINT32_1,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			1
		}
	},
	{
		ComponentFormat::F_NORMUINT32_1,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			1
		}
	},
	{
		ComponentFormat::F_FLOAT_32_1,
		ComponentInformation{
			Enum::AttributeDataType::Float32,
			1
		}
	},
	{
		ComponentFormat::F_UINT_10_10_10_L1,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			1
		}
	},
	{
		ComponentFormat::F_NORMINT_10_10_10_L1,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			1
		}
	},
	{
		ComponentFormat::F_NORMINT_11_11_10,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			1
		}
	},
	{
		ComponentFormat::F_NORMINT_10_10_10_2,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			1
		}
	},
	{
		ComponentFormat::F_UINT_10_10_10_2,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			1
		}
	},
	{
		ComponentFormat::F_INT8_2,
		ComponentInformation{
			Enum::AttributeDataType::Int8,
			2
		}
	},
	{
		ComponentFormat::F_UINT8_2,
		ComponentInformation{
			Enum::AttributeDataType::UInt8,
			2
		}
	},
	{
		ComponentFormat::F_NORMINT8_2,
		ComponentInformation{
			Enum::AttributeDataType::UInt8,
			2
		}
	},
	{
		ComponentFormat::F_NORMUINT8_2,
		ComponentInformation{
			Enum::AttributeDataType::UInt8,
			2
		}
	},
	{
		ComponentFormat::F_INT16_2,
		ComponentInformation{
			Enum::AttributeDataType::Int16,
			2
		}
	},
	{
		ComponentFormat::F_UINT16_2,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			2
		}
	},
	{
		ComponentFormat::F_NORMINT16_2,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			2
		}
	},
	{
		ComponentFormat::F_NORMUINT16_2,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			2
		}
	},
	{
		ComponentFormat::F_FLOAT16_2,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			2
		}
	},
	{
		ComponentFormat::F_UNKNOWN_20240,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			2
		}
	},
	{
		ComponentFormat::F_INT32_2,
		ComponentInformation{
			Enum::AttributeDataType::Int32,
			2
		}
	},
	{
		ComponentFormat::F_UINT32_2,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			2
		}
	},
	{
		ComponentFormat::F_NORMINT32_2,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			2
		}
	},
	{
		ComponentFormat::F_NORMUINT32_2,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			2
		}
	},
	{
		ComponentFormat::F_FLOAT32_2,
		ComponentInformation{
			Enum::AttributeDataType::Float32,
			2
		}
	},
	{
		ComponentFormat::F_INT8_3,
		ComponentInformation{
			Enum::AttributeDataType::Int8,
			3
		}
	},
	{
		ComponentFormat::F_UINT8_3,
		ComponentInformation{
			Enum::AttributeDataType::UInt8,
			3
		}
	},
	{
		ComponentFormat::F_NORMINT8_3,
		ComponentInformation{
			Enum::AttributeDataType::UInt8,
			3
		}
	},
	{
		ComponentFormat::F_NORMUINT8_3,
		ComponentInformation{
			Enum::AttributeDataType::UInt8,
			3
		}
	},
	{
		ComponentFormat::F_INT16_3,
		ComponentInformation{
			Enum::AttributeDataType::Int16,
			3
		}
	},
	{
		ComponentFormat::F_UINT16_3,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			3
		}
	},
	{
		ComponentFormat::F_NORMINT16_3,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			3
		}
	},
	{
		ComponentFormat::F_NORMUINT16_3,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			3
		}
	},
	{
		ComponentFormat::F_FLOAT16_3,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			3
		}
	},
	{
		ComponentFormat::F_INT32_3,
		ComponentInformation{
			Enum::AttributeDataType::Int32,
			3
		}
	},
	{
		ComponentFormat::F_UINT32_3,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			3
		}
	},
	{
		ComponentFormat::F_NORMINT32_3,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			3
		}
	},
	{
		ComponentFormat::F_NORMUINT32_3,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			3
		}
	},
	{
		ComponentFormat::F_FLOAT32_3,
		ComponentInformation{
			Enum::AttributeDataType::Float32,
			3
		}
	},
	{
		ComponentFormat::F_INT8_4,
		ComponentInformation{
			Enum::AttributeDataType::Int8,
			4
		}
	},
	{
		ComponentFormat::F_UINT8_4,
		ComponentInformation{
			Enum::AttributeDataType::UInt8,
			4
		}
	},
	{
		ComponentFormat::F_NORMINT8_4,
		ComponentInformation{
			Enum::AttributeDataType::UInt8,
			4
		}
	},
	{
		ComponentFormat::F_NORMUINT8_4,
		ComponentInformation{
			Enum::AttributeDataType::UInt8,
			4
		}
	},
	{
		ComponentFormat::F_NORMUINT8_4_BGRA,
		ComponentInformation{
			Enum::AttributeDataType::UInt8,
			4
		}
	},
	{
		ComponentFormat::F_INT16_4,
		ComponentInformation{
			Enum::AttributeDataType::Int16,
			4
		}
	},
	{
		ComponentFormat::F_UINT16_4,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			4
		}
	},
	{
		ComponentFormat::F_NORMINT16_4,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			4
		}
	},
	{
		ComponentFormat::F_NORMUINT16_4,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			4
		}
	},
	{
		ComponentFormat::F_FLOAT16_4,
		ComponentInformation{
			Enum::AttributeDataType::UInt16,
			4
		}
	},
	{
		ComponentFormat::F_INT32_4,
		ComponentInformation{
			Enum::AttributeDataType::Int32,
			4
		}
	},
	{
		ComponentFormat::F_UINT32_4,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			4
		}
	},
	{
		ComponentFormat::F_NORMINT32_4,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			4
		}
	},
		{
		ComponentFormat::F_NORMUINT32_4,
		ComponentInformation{
			Enum::AttributeDataType::UInt32,
			4
		}
	},
	{
		ComponentFormat::F_FLOAT32_4,
		ComponentInformation{
			Enum::AttributeDataType::Float32,
			4
		}
	}
};