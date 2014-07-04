{
    "targets": [
        {
            'target_name': 'pipedexec',
            'conditions': [
                [
                    'OS=="win"',
                    {'sources': ['win/main.c']},
                    {'sources': ['linux/main.c']}
                ],
            ]
        }
    ]
}
