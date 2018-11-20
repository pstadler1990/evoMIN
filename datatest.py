from random import randint

n = 50

j_data = ''

for i in range(0, n):

  payload_length = randint(1,32)

  payload = ''
  for r in range(0, payload_length):
    payload += '{0},'.format(randint(0x00, 0xFF))

  payload = payload[:-1]


  msg = '''uint8_t testData{0}[] = {1}0xAA,0xAA,0xAA,EVOMIN_CMD_CHIP,{2},{3},0xCC,0x55{4};'''.format(i,'{', payload_length, payload, '}')

  j_data += '{0}testData{1}{2},'.format('{', i, '}')

  # Print the single rows
  print(msg)

j_data = j_data[:-1]

jagged_array = 'uint8_t* testData[] = {0} {1} {2};'.format('{', j_data, '}')

# Print the jagged array
print(jagged_array)