#ifndef _TJSERIALIZABLE_H
#define _TJSERIALIZABLE_H

class TiXmlElement;

namespace tj {
	namespace shared {
		class EXPORTED Serializable {
			public:
				virtual ~Serializable();
				virtual void Save(TiXmlElement* parent) = 0;
				virtual void Load(TiXmlElement* you) = 0;
		};

		template<typename T> inline void SaveAttribute(TiXmlElement* parent, const char* name, const T& val) {
			std::string cs = StringifyMbs(val);
			TiXmlElement element(name);
			element.InsertEndChild(TiXmlText(cs.c_str()));
			parent->InsertEndChild(element);
		}

		template<typename T> inline T LoadAttribute(TiXmlElement* you, const char* name, const T& defaultValue) {
			TiXmlElement* elem = you->FirstChildElement(name);
			if(elem==0) {
				return defaultValue;
			}

			TiXmlNode* nd = elem->FirstChild();
			if(nd==0) return defaultValue;
			return StringTo<T>(nd->Value(), defaultValue);
		}

		template<typename T> inline void SaveAttributeSmall(TiXmlElement* parent, const char* name, const T& val) {
			std::string cs = StringifyMbs(val);
			parent->SetAttribute(name, cs.c_str());
		}

		template<typename T> inline T LoadAttributeSmall(TiXmlElement* you, const char* name, const T& defaultValue) {
			const char* value = you->Attribute(name);
			if(value!=0) {
				return StringTo<T>(value, defaultValue);
			}
			return LoadAttribute<T>(you, name, defaultValue);
		}

		class EXPORTED XML {
			public:
				static void GetElementHash(const TiXmlNode* node, SecureHash& sh);
		};

		class EXPORTED FileWriter: public virtual Object {
			public:
				FileWriter(const std::string& rootElementName);
				virtual ~FileWriter();

				void Save(const std::string& filename);
				strong<TiXmlElement> GetRoot();
				strong<TiXmlDocument> GetDocument();

				void Add(Serializable* ser);
				void Add(ref<Serializable> ser);

			protected:
				strong<TiXmlDocument> _document;
				strong<TiXmlElement> _root;
		};

		class EXPORTED FileReader: public virtual Object {
			public:
				FileReader();
				virtual ~FileReader();
				void Read(const std::string& filename, ref<Serializable> model);
		};
	}
}


#endif