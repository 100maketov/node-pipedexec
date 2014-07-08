{
    "conditions": [
        ['OS=="win"', {
            "targets": [
                {
                    "target_name": "pse",
                    "sources": ["wpse.cpp"]
                }
            ]
        },{
            "targets": [
                {
                    "target_name": "pse",
                    "sources": ["lpse.cpp"]
                }
            ]
        }]
    ]
}