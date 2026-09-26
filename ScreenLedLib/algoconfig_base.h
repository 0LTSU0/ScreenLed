#ifndef ALGOCONFIG_BASE_H
#define ALGOCONFIG_BASE_H

template <typename Struct, typename Member>
struct FieldInfo {
    const char* label;
    Member Struct::* ptr;
    double minVal = 0;
    double maxVal = 0;
    double step   = 1;
    bool hasRange = false;
};

// Basic field, no explicit range (widgets fall back to sane defaults).
#define FIELD(StructType, member, label) \
FieldInfo<StructType, decltype(StructType::member)>{label, &StructType::member}

// Field with an explicit min/max/step (useful for spin boxes).
#define FIELD_RANGE(StructType, member, label, lo, hi, stepVal) \
FieldInfo<StructType, decltype(StructType::member)>{label, &StructType::member, lo, hi, stepVal, true}

#endif // ALGOCONFIG_BASE_H
