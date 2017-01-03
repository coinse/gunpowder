def avm_search(input_vector):
    best = input_vector
    for idx in range(len(input_vector)):
        candidate = iterative_pattern_search(best, idx)
        if fitness(best) > fitness(candidate):
            best = candidate
    return best


def iterative_pattern_search(input_vector, idx):
    SCALING_FACTOR = 2
    STEP = 1

    def pattern_search(vector, index):
        direction = search_direction(vector, index)
        if direction == 0:
            return vector
        gap = direction * STEP
        better = vector
        while True:
            candidate = better[:]
            candidate[index] += gap
            gap *= SCALING_FACTOR
            if fitness(candidate) < fitness(better):
                better = candidate
            else:
                break
        return better

    def search_direction(vector, index):
        left = vector[:]
        left[index] += -1

        right = vector[:]
        right[index] += 1

        if fitness(right) < fitness(vector):
            return 1
        elif fitness(left) < fitness(vector):
            return -1
        else:
            return 0

    best = input_vector
    while True:
        candidate = pattern_search(best, idx)
        if fitness(best) > fitness(candidate):
            best = candidate
        else:
            break

    return best

#TODO: fitness(vector), caller of avm_serach(input_vector)
