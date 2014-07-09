{
    "conditions": [
        ['OS=="win"', {
            "targets": [
                {
                    "target_name": "pse",
                    "sources": ["wpse.cpp"]
                }
            ],
			"configurations": {
				"Release": {}
			}
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