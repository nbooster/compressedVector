# compressedVector
A dynamic vector like class for handling efficiently big continuous chunks of values.

If you have a huge vector that has a similar form to this:

[1,1,...,1,1,2,2,...,2,2,5,5,...,5,5,0,0,...,0]

then this class is for you !!!

Signature
```
template <BigInteger keyType = uint64_t, class valueType = int64_t, bool arithmeticSafe = false, bool threadSafe = false>
class compressedVector
```

API
```
constexpr compressedVector()

constexpr explicit compressedVector(const valueType& initValue)

constexpr compressedVector(const keyType& minIndexArg, const keyType& maxIndexArg, const valueType& initValue)

constexpr compressedVector(keyType minIndexArg, const valueType& initValue, const std::vector<std::pair<valueType, std::size_t>>& vectorValues)

constexpr compressedVector(const compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& other)

constexpr compressedVector<keyType, valueType, threadSafe>& operator=(const compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& other) & noexcept

constexpr compressedVector(compressedVector<keyType, valueType, arithmeticSafe, threadSafe>&& other)

constexpr compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& operator=(compressedVector<keyType, valueType, arithmeticSafe, threadSafe>&& other) & noexcept

constexpr void reset(const valueType& value)

[[nodiscard]] constexpr const valueType& operator[](keyType const& key) const 

[[nodiscard]] constexpr bool operator==(const compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& other) const

[[nodiscard]] constexpr const auto& getInitialValue(void) const noexcept

[[nodiscard]] constexpr std::size_t getMapSize(void) const noexcept

[[nodiscard]] constexpr std::size_t getByteSize(void) const noexcept

[[nodiscard]] constexpr keyType getMinIndex(void) const noexcept

[[nodiscard]] constexpr keyType getMaxIndex(void) const noexcept

[[nodiscard]] constexpr std::pair<keyType, keyType> getMinMaxIndex(void) const

[[nodiscard]] constexpr auto getLength(void) const noexcept

[[nodiscard]] constexpr std::pair<keyType, keyType> getValidRange(const keyType& from, const keyType& to) const

[[nodiscard]] constexpr bool contains(const keyType& fromArg, const keyType& toArg, const valueType& value) const

[[nodiscard]] constexpr bool contains(const valueType& value) const

[[maybe_unused]] constexpr bool setMinIndex(const keyType& index)

[[maybe_unused]] constexpr bool setMaxIndex(const keyType& index)

[[maybe_unused]] constexpr bool setMinMaxIndex(const keyType& minIndexArg, const keyType& maxIndexArg)

constexpr void setInitValue(const valueType& value)

constexpr void assignValue(const keyType& fromArg, const keyType& toArg, const valueType& value)

constexpr void assignValues(const std::vector<std::pair<valueType, std::size_t>>& values)

constexpr void assignValues(const keyType& fromArg, const keyType& toArg, const std::vector<std::pair<valueType, std::size_t>>& values)

constexpr void insertValuesAfterPosition(const keyType& position, const std::vector<std::pair<valueType, std::size_t>>& values,const bool expandLeft = false)

constexpr void appendValues(const std::vector<std::pair<valueType, std::size_t>>& values, const bool expandLeft = false)

constexpr void prependValues(const std::vector<std::pair<valueType, std::size_t>>& values, const bool expandLeft = false)

constexpr void erase(const keyType& fromArg, const keyType& toArg,const bool shrinkRight = false)

constexpr void shiftLeft(const keyType& difference = keyType(1))

constexpr void shiftRight(const keyType& difference = keyType(1))

constexpr void reverseValues(void)

constexpr void reverseValues(const keyType& fromArg, const keyType& toArg)

constexpr void sortValues(const bool reverse = false, const std::function<bool(const valueType&, const valueType&)>& compareFunction = std::less<valueType>{})

constexpr void sortValues(const keyType& fromArg, const keyType& toArg, const bool reverse = false, const std::function<bool(const valueType&, const valueType&)>& compareFunction = std::less<valueType>{})

constexpr void replaceValue(const valueType& oldValue, const valueType& newValue)

constexpr void replaceValue(const keyType& fromArg, const keyType& toArg, const valueType& oldValue, const valueType& newValue)

constexpr void apply(const std::function<valueType(const valueType&)>& appliedFunction)

constexpr void apply(const keyType& fromArg, const keyType& toArg, const std::function<valueType(const valueType&)>& appliedFunction)

[[nodiscard]] constexpr compressedVector<keyType, valueType, arithmeticSafe, threadSafe> getSubArray(const keyType& fromArg, const keyType& toArg) const

[[nodiscard]] constexpr std::vector<std::tuple<keyType, keyType, valueType>> getValues(const bool shrink = false) const

[[nodiscard]] constexpr std::vector<std::tuple<keyType, keyType, valueType>> getValues(const keyType& fromArg, const keyType& toArg, const bool shrink = false) const

[[nodiscard]] constexpr std::unordered_set<valueType> getSetValues(void) const

[[nodiscard]] constexpr std::unordered_set<valueType> getSetValues(const keyType& fromArg, const keyType& toArg) const

[[nodiscard]] constexpr std::map<valueType, std::size_t> getRangeValueFrequencies(void) const

[[nodiscard]] constexpr std::map<valueType, std::size_t> getRangeValueFrequencies(const keyType& fromArg, const keyType& toArg) const

[[nodiscard]] constexpr std::vector<std::pair<keyType, keyType>> getValueRanges(const valueType& value, const bool shrink = false) const

[[nodiscard]] constexpr std::unordered_map<valueType, std::vector<std::pair<keyType, keyType>>> getValuesRanges(const std::unordered_set<valueType>& values, const bool shrink = false) const

[[nodiscard]] constexpr valueType getMinValue(const std::function<bool(const valueType&, const valueType&)>& compareFunction = std::less<valueType>{}) const

[[nodiscard]] constexpr valueType getMinValue(const keyType& fromArg, const keyType& toArg, const std::function<bool(const valueType&, const valueType&)>& compareFunction = std::less<valueType>{}) const

[[nodiscard]] constexpr valueType getMaxValue(const std::function<bool(const valueType&, const valueType&e)>& compareFunction = std::less<valueType>{}) const

[[nodiscard]] constexpr valueType getMaxValue(const keyType& fromArg, const keyType& toArg, const std::function<bool(const valueType&, const valueType&)>& compareFunction = std::less{}) const

[[nodiscard]] constexpr valueType getSum(const valueType& init = valueType(0)) const

[[nodiscard]] constexpr valueType getSum(const keyType& fromArg, const keyType& toArg, const valueType& init = valueType(0)) const

[[nodiscard]] constexpr valueType getProduct(const valueType& init = valueType(1)) const

[[nodiscard]] constexpr valueType getProduct(const keyType& fromArg, const keyType& toArg, const valueType& init = valueType(1)) const

[[nodiscard]] constexpr std::vector<valueType> uncompressToVector(void) const

[[nodiscard]] constexpr std::vector<valueType> uncompressToVector(const keyType& fromArg, const keyType& toArg) const

void print(bool uncopressed = false, std::ostream& os = std::cout, const bool newLine = true, const short prec = 3) const

void print(const keyType& fromArg, const keyType& toArg, bool uncopressed = false, std::ostream& os = std::cout, const bool newLine = true, const short prec = 3) const

friend std::ostream& operator<<(std::ostream& os, const compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& array)

[[maybe_unused]] bool toFile(std::string_view filePath, const keyType& from, const keyType& to, bool append = false, bool uncopressed = false, bool trimEdges = true, const short prec = 3) const

[[nodiscard]] static constexpr compressedVector<keyType, valueType, arithmeticSafe, threadSafe> compress(const std::span<const valueType> buffer, keyType startIndex = keyType(0), const valueType& initValue = valueType(0), double* ratio = nullptr)

[[nodiscard]] static constexpr compressedVector<keyType, valueType, arithmeticSafe, threadSafe> compressFromFile(std::string_view filePath, const std::size_t indexFromStart = 0, const std::size_t indexFromEnd = 0, const valueType& initValue = valueType(0), double* ratio = nullptr)

[[nodiscard]] static constexpr compressedVector<keyType, valueType, arithmeticSafe, threadSafe> compressFromFileParts(std::string_view filePath,const std::vector<std::pair<const std::size_t, const std::size_t>>& ranges,const valueType& initValue = valueType(0), double* ratio = nullptr)
```

TODO

```
constexpr void assignRangeFromArray(compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& array, const keyType& fromA, const keyType& toA,const keyType& fromB, const keyType& toB)

constexpr void swapRangesWithArray(const keyType& fromA, const keyType& toA, compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& array, const keyType& fromB, const keyType& toB)

constexpr void swapRanges(const keyType& fromA, const keyType& toA, const keyType& fromB, const keyType& toB)

constexpr void copyRange(const keyType& fromA, const keyType& toA, const keyType& fromB, const keyType& toB)

constexpr void increaseValueRange(const keyType& from, const keyType& to, const valueType& value, const std::size_t amount, const bool expandLeft = false)

constexpr void decreaseValueRange(const keyType& from, const keyType& to, const valueType& value, const std::size_t amount, const bool shrinkRight = false)

constexpr void changeValueRange(const keyType& from, const keyType& to, const valueType& value, const std::size_t newRange)

constexpr void shiftRangeLeft(const keyType& from, const keyType& to, const keyType& difference = keyType(1), const valueType& fillValue = valueType(0))

constexpr void shiftRangeRight(const keyType& from, const keyType& to, const keyType& difference = keyType(1), const valueType& fillValue = valueType(0))

constexpr void operationWithArray(compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& array, const keyType& fromArg, const keyType& toArg, const std::function<valueType(const valueType&, const valueType&)>& operation)

[[nodiscard]] constexpr std::vector<std::tuple<keyType, keyType, bool>> compareWithArray(compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& array, const keyType& fromArgLeft, const keyType& toArgLeft, const keyType& fromArgRight, const keyType& toArgRight, const std::function<bool(const valueType&, const valueType&)>& compareFunction = std::equal_to<valueType>{}) const
```
