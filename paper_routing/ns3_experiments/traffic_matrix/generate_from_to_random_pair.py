# Author: silent-rookie     2024

import random

# source to destination
# for example: 
# NA to : NA, SA, Europe, Africa, Asia, Oceania
# SA to : ...
NorthAmerica_rate = [0.6, 0.1, 0.15, 0.02, 0.1, 0.03]
SouthAmerica_rate = [0.35, 0.4, 0.12, 0.02, 0.08, 0.03]
Europe_rate = [0.4, 0.05, 0.4, 0.02, 0.1, 0.03]
Africa_rate = [0.4, 0.02, 0.3, 0.2, 0.05, 0.03]
Asia_rate = [0.3, 0.02, 0.1, 0.02, 0.5, 0.06]
Oceania_rate = [0.4, 0.02, 0.1, 0.02, 0.12, 0.34]

# top 100 city
# Each continent city rate: 0.16, 0.1, 0.07, 0.1, 0.55, 0.02
NorthAmerica = [5, 9, 20, 37, 56, 57, 60, 62, 64, 70, 74, 
                76, 83, 88, 92, 94]
SouthAmerica = [3, 12, 18, 29, 32, 50, 59, 90, 97, 98]
Europe = [14, 21, 24, 27, 54, 69, 73]
Africa = [8, 16, 22, 34, 66, 71, 72, 75, 78, 96]
Asia = [0, 1, 2, 4, 6, 7, 10, 11, 13, 15, 17, 19, 23, 25, 26, 
        28, 30, 31, 33, 35, 36, 38, 39, 40, 41, 42, 43, 
        44, 45, 46, 47, 48, 49, 51, 52, 53, 55, 58, 61, 63, 
        65, 67, 68, 77, 79, 80, 81, 82, 85, 86, 87, 91, 93, 95, 99]
Oceania = [84, 89]

city_rate = [0.16, 0.1, 0.07, 0.1, 0.55, 0.02]



def generate_from_to_random_pair_top100_city(pair_num, random_num, seed, num_satellites=1584):
    '''
    pair_num:       the num of pair generate
    random_num:     the num of random pair generate
    seed:           random seed
    num_satellites: num of satellites, for ground station offset
                    for convinent, now we set it as 1584

    According to paper, we generate pair_num pairs which consistent with "source to destinatino" rate,
    and generate random_num pairs randomly

    return:     generate list with (pair_num + random_num) pairs
    '''

    # Set random seed
    random.seed(seed)

    pairs = []

    for i in range(pair_num):
        pair_from = random.randint(0, 99)
        pair_to = generate_random_city_from_to(pair_from)
        pairs.append((pair_from + num_satellites, pair_to + num_satellites))
    
    for i in range(random_num):
        pair_from = random.randint(0, 99)
        pair_to = pair_from
        while(pair_to == pair_from):
            pair_to = random.randint(0, 99)
        pairs.append((pair_from + num_satellites, pair_to + num_satellites))
    
    return pairs

def generate_random_city_from_to(pair_from):
    if pair_from in NorthAmerica:
        return generate_random_to_city(pair_from, NorthAmerica_rate)
    elif pair_from in SouthAmerica:
        return generate_random_to_city(pair_from, SouthAmerica_rate)
    elif pair_from in Europe:
        return generate_random_to_city(pair_from, Europe_rate)
    elif pair_from in Africa:
        return generate_random_to_city(pair_from, Africa_rate)
    elif pair_from in Asia:
        return generate_random_to_city(pair_from, Asia_rate)
    elif pair_from in Oceania:
        return generate_random_to_city(pair_from, Oceania_rate)
    else:
        raise ValueError("invalid pair_from: %d" % pair_from)
    
def generate_random_to_city(pair_from, rates):
    tmp_rates = []
    for i in range(len(rates)):
        tmp_rates.append(sum(rates[0 : i+1]))
    
    target_city = []

    tmp = random.random()
    if tmp < tmp_rates[0]:
        target_city = NorthAmerica
    elif tmp < tmp_rates[1]:
        target_city = SouthAmerica
    elif tmp < tmp_rates[2]:
        target_city = Europe
    elif tmp < tmp_rates[3]:
        target_city = Africa
    elif tmp < tmp_rates[4]:
        target_city = Asia
    elif tmp < tmp_rates[5]:
        target_city = Oceania
    else:
        raise ValueError("invalid tmp value: %d" % tmp)
    
    while True:
        tmp_city = random.sample(target_city, 1)[0]
        if(tmp_city != pair_from):
            return tmp_city

def generate_pairs_logging_ids(pairs):
    res = "set("
    for i in range(len(pairs)):
        res += str(i)
        if i != len(pairs) - 1:
            res += ","
    res += ")"
    return res