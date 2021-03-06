fr = open("sort", "r")
num = 0 
flag = 0
limit = 24
max_count = 1 
result = {} 
for line in fr.readlines():
    
    if num == max_count:
        break
    else:
        num += 1

    #phase one
    data = line[:-1].split()
    #print len(data), data[0], data[1], data[2]
    key = data[0]
    
    current_len = 0
    current_value = ""
    data_len = 0 
    data_sha = ""
    data_value = ""
    
    #phase two 
    f_key = open("char_get", "r")
    for line_key in f_key.readlines():
        data = line_key[:-1]
    
        if flag == 0:
            data_len = int(data.split("|")[1])
        elif flag == 1:
            data_sha = data
        elif flag == 2:
            data_value = data
        else:
            print 'error case'
            exit(-1)
        
        if flag == 2:
            flag = 0

            if data_sha == key:
                current_len = data_len 
                current_value = data_value 
                #print 'now case', key, current_len, current_value
                break;
        else:
            flag += 1
    f_key.close()

    #phase three
    look_value = current_value.split(":")[:-1]
    #print look_value 
    #key:string value:count
    look_map = {}       
    look_result = []
    look_result.append(current_value)

    if current_len <= limit:
        continue
    
    j = 0
    f_key = open("char_get", "r")
    for step in range(current_len - 1, limit - 1, -1):
            #print 'step is', step, len(look_value)

            for dummy_i in range(current_len - step + 1):
                #print 'dummy_i is', dummy_i
                look_key = ""
                f_key.seek(0)
                for i in range(dummy_i, dummy_i + step):
                    look_key += look_value[i] + ":"
                
                print j, look_key
                j += 1
            
                # maybe Palindrome
                if look_key not in look_map:
                    for line in f_key.readlines():
                        if line[:-1].find(look_key) != -1:
                            if line[:-1] not in look_result:
                                look_result.append(line[:-1])
                                if look_key not in look_map:
                                    look_map[look_key] = 1 
                                else:
                                    look_map[look_key] += 1 
    f_key.close()
                    
    result[key] = 0
    for key_map in look_map:
        #print key, len(key) * look_map[key]
        result[key] += len(key_map) * look_map[key_map]

    print key, result[key]
    

fr.close()

all_sum = 0
for key in result:
    #print key, result[key]
    all_sum += result[key]
print all_sum
