#ifndef CSVReader_H
#define CSVReader_H

#include <iostream>
#include <fstream>
#include <string>
#include <type_traits>
#include <sstream>
#include <array>
#include <functional>
#include <exception>
#include <stdexcept>
#include <utility>

/*

TODO:
* TO-DO better base-classing
* --> Remove Copy & Pasta
*   --> Use base class for parser (HasNext, NextLine)?
/
*/

namespace CSVReader {



    // ========= Exceptions START ===========
    class parse_error : public std::logic_error
    {
    private:
        int _line;
        int _field;
    public:
        explicit parse_error(const char* msg, int line, int field) : std::logic_error(msg), _line(line), _field(field) {}
        explicit parse_error(std::string msg, int line, int field) : parse_error(msg.c_str(), line, field) {}

        inline int get_line_number()
        {
            return _line;
        }
        inline int get_field_number()
        {
            return _field;
        }
    };
    // ========= Exceptions END ===========



    // ========= Utils START ===========
    // This is a feature built-in for C++14
    namespace detail {
        template <std::size_t... I>
        class index_sequence {};

        template <std::size_t N, std::size_t ...I>
        struct make_index_sequence : make_index_sequence<N-1, N-1,I...> {};

        template <std::size_t ...I>
        struct make_index_sequence<0,I...> : index_sequence<I...> {};
    }

    // This is for the CSVBatchReader
    // This template class depends on .push_back()
    template<class ContainerValueT, class ContainerT>
    struct CommonContainerUtils
    {
        static void Add(ContainerT* container, const ContainerValueT& value){
            container->push_back(value);
        }
    };

    namespace detail {

        template<class StrT,
                 class CharT,
                 class StrTUtils,
                 class Converter>
        class CSVReaderBase {
        protected:
            size_t _currentCharIndex;
            CharT _sep;
            StrT _currentLine; // Will be written by the derived class
            int _fieldTracker; // Will be written by the derived class
            int _lineTracker;  // Will be written by the derived class

            CSVReaderBase(CharT sep) : _currentCharIndex(0u), _sep(sep),
                _currentLine(""), _fieldTracker(0), _lineTracker(0) {}

            inline StrT NextField()
            {
                size_t newCharIndex = _currentCharIndex;
                if (!StrTUtils::find(_currentLine, _sep, newCharIndex))
                    newCharIndex = StrTUtils::length(_currentLine);

                StrT next = StrTUtils::substring(_currentLine, _currentCharIndex, newCharIndex - _currentCharIndex);
                _currentCharIndex = newCharIndex + 1;
                return next;
            }

            inline void SkipField()
            {
                size_t newCharIndex = _currentCharIndex;
                if (!StrTUtils::find(_currentLine, _sep, newCharIndex))
                    newCharIndex = StrTUtils::length(_currentLine);
                _currentCharIndex = newCharIndex + 1;
            }

            inline bool HasNext()
            {
                return _currentCharIndex <= StrTUtils::length(_currentLine);
            }

            template<typename ToType>
            inline void SafeConvert(ToType* to, const StrT& from)
            {
                try
                {
                    Converter::Convert(to, from);
                }
                catch (...)
                {
                    ThrowParseErrorInCatchContext();
                }
            }

            inline void ThrowParseErrorInCatchContext()
            {
                std::throw_with_nested(parse_error(std::string("Failed to parse field ") + std::to_string(_fieldTracker) + " at line " + std::to_string(_lineTracker), _lineTracker, _fieldTracker));
            }
        };
    }
    // ========= Utils END ===========



    // ========= Readers START ===========
    template<class StrElementType, class StrElementTraits, class StrElementAlloc = std::allocator<StrElementType> >
    struct IfStreamReader
    {
        using target_ifstream = std::basic_ifstream<StrElementType, StrElementTraits>;
        using target_string = std::basic_string<StrElementType, StrElementTraits, StrElementAlloc>;
        typedef target_string string_type;

        target_ifstream* _reader;
        IfStreamReader(target_ifstream* reader) : _reader(reader) {}

        target_string read_line()
        {
            target_string str;
            std::getline(*_reader, str);
            return str;
        }
    };
    template<class StrElementType, class StrElementTraits>
    constexpr IfStreamReader<StrElementType, StrElementTraits> MakeIfStreamReader(std::basic_ifstream<StrElementType, StrElementTraits>* reader)
    {
        return IfStreamReader<StrElementType, StrElementTraits>(reader);
    }

    template<class StrElementType, class StrElementTraits, class StrElementAlloc>
    struct StringReader
    {
        using target_istringstream = std::basic_istringstream<StrElementType, StrElementTraits, StrElementAlloc>;
        using target_string = std::basic_string<StrElementType, StrElementTraits, StrElementAlloc>;
        typedef target_string string_type;

        target_istringstream _txt;
        StringReader(target_string str) : _txt(str) {}

        target_string read_line()
        {
            target_string str;
            std::getline(_txt, str);
            return str;
        }
    };
    template<class StrElementType, class StrElementTraits, class StrElementAlloc>
    constexpr StringReader<StrElementType, StrElementTraits, StrElementAlloc> MakeStringReader(const std::basic_string<StrElementType, StrElementTraits, StrElementAlloc>& str)
    {
        return StringReader<StrElementType, StrElementTraits, StrElementAlloc>(str);
    }

    template<class StrT>
    struct DirectReader
    {
        DirectReader(const StrT& data) : _data(data) {}
        StrT read_line()
        {
            return _data;
        }
    private:
        StrT _data;
    };
    template<class StrT>
    constexpr DirectReader<StrT> MakeDirectReader(const StrT& data){
        return DirectReader<StrT>(data);
    }

    // ========= Readers END ===========







    // ========= Special Attributes START ===========
    // 1. Alternative Parameter - CSVDiscard
    struct CSVDiscard {};

    // 2. Alternative Parameter - CSVValidator
    template<typename T>
    struct CSVValidator {
        typedef T value_type;
        CSVValidator(T* value, std::function<bool(const T&)> validatorFunction) : _value(value), _validatorFunction(validatorFunction) {}

        bool Validate() const {
            return _validatorFunction(_value);
        }
        T* Get() {
            return _value;
        }
    private:
        T* _value;
        std::function<bool(const T&)> _validatorFunction;
    };
    template<typename T, typename ValidatorFunc>
    constexpr CSVValidator<T> MakeCSVValidator(T* value, ValidatorFunc validatorFunction) {
        return CSVValidator<T>(value, validatorFunction);
    }

    // 3. Alternative Parameter - CSVPostProcessor
    template<typename T>
    struct CSVPostProcessor
    {
        typedef T value_type;

        CSVPostProcessor(T* value, const std::function<void(T&)>& postProcessorFunction) : CSVPostProcessor(value, postProcessorFunction, nullptr) {}
        CSVPostProcessor(T* value, const std::function<void(T&)>& postProcessorFunction, const std::function<bool(const T&)>& validatorFunction)
            : _value(value), _validatorFunction(validatorFunction), _postProcessorFunction(postProcessorFunction) {}

        bool Validate() const {
            if (_validatorFunction)
                return _validatorFunction(*_value);
            return true;
        }
        inline void PostProcess()
        {
            _postProcessorFunction(*_value);
        }
        T* Get() {
            return _value;
        }
    private:
        T* _value;
        std::function<bool(const T&)> _validatorFunction;
        std::function<void(T&)> _postProcessorFunction;
    };
    template<typename T, typename PostProcessorFunc>
    constexpr CSVPostProcessor<T> MakeCSVPostProcessor(T* value, PostProcessorFunc postProcessorFunction)
    {
        return CSVPostProcessor<T>(value, postProcessorFunction);
    }
    template<typename T, typename PostProcessorFunc, typename ValidatorFunc>
    constexpr CSVPostProcessor<T> MakeCSVPostProcessor(T* value, PostProcessorFunc postProcessorFunction, ValidatorFunc validatorFunction)
    {
        return CSVPostProcessor<T>(value, postProcessorFunction, validatorFunction);
    }



    // 4. Alternative parameter - CSVOptional
    template<typename T>
    struct CSVOptional
    {
        typedef T value_type;

        CSVOptional(T* value, const T defVal) : CSVOptional(value, defVal, nullptr) {}
        CSVOptional(T* value, const T defVal, const std::function<bool(const T&)>& validatorFunction) :
            _value(value), _defaultValue(defVal), _validatorFunction(validatorFunction) {}
        CSVOptional(T* value, const T defVal, const std::function<bool(const T&)>& validatorFunction, const std::function<void(T&)>& postProcessorFunction) :
            _value(value), _defaultValue(defVal), _validatorFunction(validatorFunction), _postProcessorFunction(postProcessorFunction) {}


        inline bool Validate() const
        {
            if (_validatorFunction)
                return _validatorFunction(*_value);
            return true;
        }
        inline void PostProcess()
        {
            if (_postProcessorFunction)
                _postProcessorFunction(*_value);
        }
        inline void AssignDefault()
        {
            *_value = _defaultValue;
        }
        inline T* Get() {
            return _value;
        }
    private:
        T* _value;
        T _defaultValue;
        std::function<bool(const T&)> _validatorFunction;
        std::function<void(T&)> _postProcessorFunction;
    };
    template<typename T>
    constexpr CSVOptional<T> MakeCSVOptional(T* value, T defVal) {
        return CSVOptional<T>(value, defVal);
    }
    template<typename T, typename ValidatorFunc>
    constexpr CSVOptional<T> MakeCSVOptional(T* value, T defVal, ValidatorFunc validatorFunction) {
        return CSVOptional<T>(value, defVal, validatorFunction);
    }
    template<typename T, typename ValidatorFunc, typename PostProcessorFunc>
    constexpr CSVOptional<T> MakeCSVOptional(T* value, T defVal, ValidatorFunc validatorFunction, PostProcessorFunc postProcessorFunction) {
        return CSVOptional<T>(value, defVal, validatorFunction, postProcessorFunction);
    }

    // 5. Reader in Reader - CSVOptional
    template<class Reader, class StrT, class CharT, class StrTUtils, class Converter, class... RestValues>
    struct CSVSubReader;

    // 6. Reading in container
    template<class ContainerValueT,
             class Container,
             class ContainerUtils,
             class StrT,
             class CharT,
             class StrTUtils,
             class Converter>
    struct CSVBatchReader : detail::CSVReaderBase<StrT, CharT, StrTUtils, Converter> {
    private:
        Container* _container;
        std::function<void(ContainerValueT&)> _postProcessorFunction;
    public:

        CSVBatchReader(CharT sep, Container* container, const std::function<void(ContainerValueT&)>& postProcessorFunction) :
            detail::CSVReaderBase<StrT, CharT, StrTUtils, Converter>(sep), _container(container), _postProcessorFunction(postProcessorFunction) {}

        inline void ReadDataLine(const StrT& val)
        {
            this->_currentLine = val;
            while(this->HasNext()){
                StrT from = this->NextField();
                if(from == "")
                    continue;
                ContainerValueT to;
                this->SafeConvert(&to, from);
                if(_postProcessorFunction)
                    _postProcessorFunction(to);
                ContainerUtils::Add(_container, to);
            }
        }
    };

    // 7. Reading with iteration
    template<class StrT,
             class CharT,
             class StrTUtils,
             class Converter>
    struct CSVIterator : detail::CSVReaderBase<StrT, CharT, StrTUtils, Converter>
    {
    private:
        std::function<void(const StrT&)> _iteratorFunc;
    public:
        CSVIterator(CharT sep, std::function<void(const StrT&)> iteratorFunc) :
            detail::CSVReaderBase<StrT, CharT, StrTUtils, Converter>(sep), _iteratorFunc(iteratorFunc) {}

        inline void ReadDataLine(const StrT& val)
        {
            this->_currentLine = val;
            while(this->HasNext()){
                StrT next = this->NextField();
                if(!(next == ""))
                    _iteratorFunc(next);
            }
        }
    };



    // ========= Special Attributes END ===========



    template<typename T>
    struct identity { typedef T type; };

    /*
    * The Default CSV Converter uses the STL library to do the most of the conversion.
    */
    template<class StrType>
    struct DefaultCSVConverter
    {
        template<typename T>
        static void Convert(T* out, const StrType& field)
        {
            ConvertInternal(out, field, identity<T>());
        }

    private:
        template<typename T>
        static void ConvertInternal(T* out, const StrType& field, identity<T>)
        {
            static_assert(!std::is_integral<T>::value, "No default converter for this type");
        }
        static void ConvertInternal(double* out, const StrType& field, identity<double>)
        {
            *out = std::stod(field);
        }
        static void ConvertInternal(float* out, const StrType& field, identity<float>)
        {
            *out = std::stof(field);
        }
        static void ConvertInternal(int* out, const StrType& field, identity<int>)
        {
            *out = std::stoi(field);
        }
        static void ConvertInternal(long* out, const StrType& field, identity<long>)
        {
            *out = std::stol(field);
        }
        static void ConvertInternal(long long* out, const StrType& field, identity<long long>)
        {
            *out = std::stoll(field);
        }
        static void ConvertInternal(long double* out, const StrType& field, identity<long double>)
        {
            *out = std::stold(field);
        }
        static void ConvertInternal(unsigned long* out, const StrType& field, identity<unsigned long>)
        {
            *out = std::stoul(field);
        }
        static void ConvertInternal(unsigned long long* out, const StrType& field, identity<unsigned long long>)
        {
            *out = std::stoull(field);
        }
        static void ConvertInternal(StrType* out, const StrType& field, identity<StrType>)
        {
            *out = field;
        }
    };



    template<class StrElementType, class StrElementTraits, class StrElementAlloc>
    struct DefaultStringWrapper
    {
        using target_string = std::basic_string<StrElementType, StrElementTraits, StrElementAlloc>;

        static bool find(const target_string& str, StrElementType sep, size_t& findIndex)
        {
            size_t pos = str.find(sep, findIndex);
            if (pos == target_string::npos)
                return false;
            findIndex = pos;
            return true;
        }

        static size_t length(const target_string& str)
        {
            return str.length();
        }

        static target_string substring(const target_string& str, size_t pos, size_t count)
        {
            return str.substr(pos, count);
        }
    };



    /*
    * This class reads CSV-Files very efficient.
    *
    * Reader is a class, which has one function member called:
    *
    *      std::string read_line();
    *
    *
    * Converter is a class, which has one static template member function called:
    *
    *      template<typename T>
    *      static void Convert(T* out, const std::string& field);
    *
    * Converter is allowed to throw std::invalid_argument error, if a conversion fails.
    *
    *
    */
    template<class Reader, class StrT, class CharT, class StrTUtils, class Converter>
    class CSVReader : detail::CSVReaderBase<StrT, CharT, StrTUtils, Converter>
    {
    private:
        Reader* _reader;
        int _currentTotalFields;
        bool _requireReadLine;
    public:
        CSVReader(Reader* reader, CharT sep) : detail::CSVReaderBase<StrT, CharT, StrTUtils, Converter>(sep), _reader(reader),
            _currentTotalFields(0), _requireReadLine(true) {}
        CSVReader(const CSVReader& other) = delete;
        CSVReader(CSVReader&& other) = default;
        ~CSVReader() = default;

    private:
        inline void ThrowIfOutOfBounds()
        {
            if (this->_currentCharIndex > StrTUtils::length(this->_currentLine))
                throw parse_error("Expected " + std::to_string(this->_currentTotalFields) + " CSV-Fields, got "
                                  + std::to_string(this->_fieldTracker) + " at line "
                                  + std::to_string(this->_lineTracker) + "!", this->_fieldTracker, this->_lineTracker);
        }
    public:
        template<class T, class... RestValues>
        void ReadNext(T nextVal, RestValues... restVals)
        {
            static_assert(std::is_pointer<T>::value, "All values which are unpacked must be pointers (except CSVDiscard, CSVVaildate, CSVDiscard, CSVOptional, CSVSubReader)!");
            ThrowIfOutOfBounds();

            //Here do conversion code
            this->SafeConvert(nextVal, this->NextField());

            this->_fieldTracker++;
            ReadNext(restVals...);
        }

        template<class... RestValues>
        void ReadNext(CSVDiscard, RestValues... restVals)
        {
            this->_fieldTracker++;
            this->SkipField();
            ReadNext(restVals...);
        }

        template<class ValidateT, class... RestValues>
        void ReadNext(CSVValidator<ValidateT> nextVal, RestValues... restVals)
        {
            ThrowIfOutOfBounds();

            this->SafeConvert(nextVal, this->NextField());

            if (!nextVal.Validate())
                throw std::logic_error("Validation failed at field " + std::to_string(this->_fieldTracker) + " at line " + std::to_string(this->_lineTracker) + "!");

            this->_fieldTracker++;
            ReadNext(restVals...);
        }

        template<class PostProcessorT, class... RestValues>
        void ReadNext(CSVPostProcessor<PostProcessorT> nextVal, RestValues... restVals)
        {
            ThrowIfOutOfBounds();

            this->SafeConvert(nextVal.Get(), this->NextField());
            if (!nextVal.Validate())
                throw std::logic_error("Validation failed at field " + std::to_string(this->_fieldTracker) + " at line " + std::to_string(this->_lineTracker) + "!");
            nextVal.PostProcess();

            this->_fieldTracker++;
            ReadNext(restVals...);
        }

        template<class OptionalT, class... RestValues>
        void ReadNext(CSVOptional<OptionalT> optionalObj, RestValues... restVals)
        {
            // If we already reached the end, then assign default
            if (this->_currentCharIndex >= StrTUtils::length(this->_currentLine)) {
                optionalObj.AssignDefault();
            }
            else
            {
                this->SafeConvert(optionalObj.Get(), this->NextField());
                if (!optionalObj.Validate())
                    throw std::logic_error("Validation failed at field " + std::to_string(this->_fieldTracker) + " at line " + std::to_string(this->_lineTracker) + "!");
                optionalObj.PostProcess();
            }

            this->_fieldTracker++;
            ReadNext(restVals...);
        }

        template<class... Args, class... RestValues>
        void ReadNext(CSVSubReader<Args...> subReaderObj, RestValues... restVals)
        {
            ThrowIfOutOfBounds();

            try
            {
                subReaderObj.ReadDataLine(this->NextField());
            }
            catch(...)
            {
                this->ThrowParseErrorInCatchContext();
            }

            this->_fieldTracker++;
            ReadNext(restVals...);
        }

        template<class... Args, class... RestValues>
        void ReadNext(CSVBatchReader<Args...> subBatchReaderObj, RestValues... restVals)
        {
            ThrowIfOutOfBounds();

            try
            {
                subBatchReaderObj.ReadDataLine(this->NextField());
            }
            catch(...)
            {
                this->ThrowParseErrorInCatchContext();
            }

            this->_fieldTracker++;
            ReadNext(restVals...);
        }

        template<class... Args, class... RestValues>
        void ReadNext(CSVIterator<Args...> iteratorObj, RestValues... restVals)
        {
            ThrowIfOutOfBounds();

            try
            {
                iteratorObj.ReadDataLine(this->NextField());
            }
            catch(...)
            {
                this->ThrowParseErrorInCatchContext();
            }

            this->_fieldTracker++;
            ReadNext(restVals...);
        }

        void ReadNext() {}

        template<typename... Values>
        CSVReader& ReadDataLine(Values... allValues)
        {
            this->_lineTracker++;
            this->_currentCharIndex = 0;
            _currentTotalFields = sizeof...(allValues);
            this->_fieldTracker = 0; // We need the tracker at 0 (because of out of range exception)
            if (_requireReadLine)
                this->_currentLine = _reader->read_line();
            ReadNext(allValues...);
            _requireReadLine = true;


            return *this;
        }

        // Begins with 1
        template<typename T>
        T ReadField(int fieldNum)
        {
            if (_requireReadLine)
                this->_currentLine = _reader->read_line();
            _requireReadLine = false;
            this->_currentCharIndex = 0;

            StrT field;
            for (int i = 1; i < fieldNum; i++) {
                if (this->_currentCharIndex >= StrTUtils::length(this->_currentLine) )
                    throw std::logic_error("Expected " + std::to_string(fieldNum) + " CSV-Fields, got " + std::to_string(i - 1) + " @ line " + std::to_string(this->_lineTracker) + "!");

                this->SkipField();
            }
            field = this->NextField();

            T value;
            Converter::Convert(&value, field);
            return value;
        }

    };

    template<class StrT, class StrTUtils, class Converter, class Reader, class CharT>
    constexpr CSVReader<Reader, StrT, CharT, StrTUtils, Converter> MakeCSVReader(Reader* reader, CharT sep)
    {
        return CSVReader<Reader, StrT, CharT, StrTUtils, Converter>(reader);
    }


    namespace detail {
        template<class Reader>
        struct CSVReaderFromReaderType
        {
            typedef typename Reader::string_type string_type;
            typedef typename string_type::value_type value_type;
            typedef typename string_type::traits_type traits_type;
            typedef typename string_type::allocator_type allocator_type;

            typedef CSVReader<Reader, string_type, value_type,
                DefaultStringWrapper<value_type, traits_type, allocator_type>, DefaultCSVConverter<string_type>> full_type;
        };
    }

    template<class Reader, class CharT>
    constexpr typename detail::CSVReaderFromReaderType<Reader>::full_type MakeCSVReaderFromBasicString(Reader* reader, CharT sep)
    {
        typedef detail::CSVReaderFromReaderType<Reader> csv_reader_type;
        typedef typename csv_reader_type::value_type value_type;

        static_assert(std::is_same<CharT, value_type>::value, "Value type of basic_string must be the same as the type of the seperator!");
        return csv_reader_type::full_type(reader, sep);
    }


    template<class Reader, class StrT, class CharT, class StrTUtils, class Converter, class... Values>
    struct CSVSubReader
    {
    public:
        CSVSubReader(CharT sep, Values... allValues) : _sep(sep), _val(allValues...)
        {}

        void ReadDataLine(const StrT& val)
        {
            ReadDataLineImpl(val, detail::make_index_sequence<sizeof...(Values)>{});
        }

    private:
        template<std::size_t ...I>
        void ReadDataLineImpl(const StrT& val, detail::index_sequence<I...>)
        {
            DirectReader<StrT> subReader(val);
            CSVReader<decltype(subReader), StrT, CharT, StrTUtils, Converter> subCSVReader(&subReader, _sep);
            subCSVReader.ReadDataLine(std::get<I>(_val)...);
        }

        CharT _sep;
        std::tuple<Values...> _val;
    };


    template<class Reader, class StrT, class CharT, class StrTUtils, class Converter, class SubChar, class... RestValues>
    constexpr CSVSubReader<Reader, StrT, CharT, StrTUtils, Converter, RestValues...> MakeCSVSubReader(const CSVReader<Reader, StrT, CharT, StrTUtils, Converter>&, SubChar sep, RestValues... values)
    {
        return CSVSubReader<Reader, StrT, CharT, StrTUtils, Converter, RestValues...>(sep, values...);
    }


    namespace detail
    {
        template<class ContainerT,                                                          // The container type
                 class StrT,                                                                // The string class
                 class CharT,                                                               // The char type
                 class StrTUtils,                                                           // The string util class
                 class Converter>
        struct CSVBatchReaderFromContainer
        {
            typedef typename ContainerT::value_type ContainerValueT;
            typedef CommonContainerUtils<ContainerValueT, ContainerT> ContainerUtils;

            typedef CSVBatchReader<ContainerValueT, ContainerT, ContainerUtils, StrT, CharT, StrTUtils, Converter> full_type;
        };
    }

    template<class ContainerT,                                                          // The container type
             class Reader,                                                              // The reader (not used)
             class StrT,                                                                // The string class
             class CharT,                                                               // The char type
             class StrTUtils,                                                           // The string util class
             class Converter>                                                           // The value converter
    constexpr typename detail::CSVBatchReaderFromContainer<ContainerT, StrT, CharT, StrTUtils, Converter>::full_type
        MakeCSVBatchReader(const CSVReader<Reader, StrT, CharT, StrTUtils, Converter>&, CharT sep, ContainerT* container)
    {
        typedef detail::CSVBatchReaderFromContainer<ContainerT, StrT, CharT, StrTUtils, Converter> csv_batch_reader_type;

        return typename csv_batch_reader_type::full_type(sep, container, nullptr);
    }

    template<class ContainerT,                                                          // The container type
             class Reader,                                                              // The reader (not used)
             class StrT,                                                                // The string class
             class CharT,                                                               // The char type
             class StrTUtils,                                                           // The string util class
             class Converter,                                                           // The value converter
             class PostProcessorFunc>
    constexpr typename detail::CSVBatchReaderFromContainer<ContainerT, StrT, CharT, StrTUtils, Converter>::full_type
        MakeCSVBatchReader(const CSVReader<Reader, StrT, CharT, StrTUtils, Converter>&, CharT sep, ContainerT* container, const PostProcessorFunc& postProcessorFunc)
    {
        typedef detail::CSVBatchReaderFromContainer<ContainerT, StrT, CharT, StrTUtils, Converter> csv_batch_reader_type;

        return typename csv_batch_reader_type::full_type(sep, container, postProcessorFunc);
    }



    template<class Reader, class StrT, class CharT, class StrTUtils, class Converter, class SubChar, class IteratorFunc>
    constexpr CSVIterator<StrT, CharT, StrTUtils, Converter>
        MakeCSVIterator(const CSVReader<Reader, StrT, CharT, StrTUtils, Converter>&, SubChar sep, const IteratorFunc& iteratorFunc)
    {
        return CSVIterator<StrT, CharT, StrTUtils, Converter>(sep, iteratorFunc);
    }
}




#endif
