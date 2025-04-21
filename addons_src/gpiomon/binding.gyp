{
    "targets": [
        {
            "target_name": "gpiomon_arm64",
            "cflags!": [ "-fno-exceptions"],
            "cflags_cc!": [ "-fno-exceptions"],
            'OTHER_CFLAGS': ["-std=c++17"],
            "sources": [ 
                "node/node.cpp", 
                "src/c_gpio.cpp",
                "src/error_code.cpp"
                ],
            "include_dirs": [ "<!@(node -p \"require('node-addon-api').include\")", "src", "node" ],
            "defines": [ "NAPI_DISABLE_CPP_EXEPTIONS" ],
        }
    ]
}
