import qbs

Product {
    name: "Basic Reflection"
    cpp.cxxLanguageVersion: "c++1z"
    files: [
        "*.hpp",
        "meta_utils/*",
        "reflect_information/*",
        "reflect_information/types/*",
        "reflect_information/variables/*",
        "reflect_information/functions/*",
        "reflect_utils/*"
    ]
    Depends { name: "cpp" }
    Export {
            Depends { name: "cpp" }
            cpp.includePaths: base.concat(product.sourceDirectory)
            cpp.cxxLanguageVersion: "c++1z"
            cpp.cxxFlags: base.concat("-Wno-non-template-friend")
    }
}
