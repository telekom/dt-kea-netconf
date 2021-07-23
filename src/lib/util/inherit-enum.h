// -- InheritEnum.h
// https://www.codeproject.com/Articles/16150/Inheriting-a-C-enum-type

#ifndef UTIL_INHERIT_ENUM
#define UTIL_INHERIT_ENUM

template <typename EnumT, typename BaseEnumT>
struct InheritEnum {
    InheritEnum() = default;

    InheritEnum(EnumT e) : enum_(e) {
    }

    InheritEnum(BaseEnumT e) : baseEnum_(e) {
    }

    explicit InheritEnum(int val) : enum_(static_cast<EnumT>(val)) {
    }

    operator EnumT() const {
        return enum_;
    }

private:
    // Note - the value is declared as a union mainly for as a debugging aid. If
    // the union is undesired and you have other methods of debugging, change it
    // to either of EnumT and do a cast for the constructor that accepts BaseEnumT.
    union {
        EnumT enum_;
        BaseEnumT baseEnum_;
    };
};

#endif  // UTIL_INHERIT_ENUM
