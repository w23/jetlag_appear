def FlagsForFile(filename, **kwargs):
    return {'flags': ['-Werror', '-Wextra', '-pedantic', 
                      '-std=c++11',
                      '-I../atto']}
