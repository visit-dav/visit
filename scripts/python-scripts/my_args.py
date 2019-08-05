
input_map = {
    'my_python_1' : ['d','p'],
    'my_python_2' : ['my_expr'],
    'my_python_3' : ['my_python_2'],
}

expressions = [
    'my_python_1 = d*p',
    'my_python_2 = np.square(my_expr) - my_expr',
    'my_python_3 = np.square(my_python_2)',
]

output_file_name = 'example.py'

