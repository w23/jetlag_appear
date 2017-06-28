def FlagsForFile(filename, **kwargs):
    if '.cc' in filename:
        std = 'c++11'
    else:
        std = 'c99'
    return {'flags': ['-Werror', '-Wextra', '-pedantic', 
                      '-std=' + std,
                      '-I../atto']}
