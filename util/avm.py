import random
from evaluation import fitness

def avm_search(TARGET, input_vector):
    best = input_vector
    for idx in range(len(input_vector)):
        candidate = iterative_pattern_search(TARGET, best, idx)
        if fitness(TARGET, best) > fitness(TARGET, candidate):
            best = candidate
    return [best, fitness(TARGET, best)]


def iterative_pattern_search(target, input_vector, idx):
    SCALING_FACTOR = 2
    STEP = 1
    TARGET = target

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
            if fitness(TARGET, candidate) < fitness(TARGET, better):
                better = candidate
            else:
                break
        return better

    def search_direction(vector, index):
        left = vector[:]
        left[index] += -1

        right = vector[:]
        right[index] += 1

        if fitness(TARGET, right) < fitness(TARGET, vector):
            return 1
        elif fitness(TARGET, left) < fitness(TARGET, vector):
            return -1
        else:
            return 0

    best = input_vector
    while True:
        candidate = pattern_search(best, idx)
        if fitness(TARGET, best) > fitness(TARGET, candidate):
            best = candidate
        else:
            break

    return best


def main():
    branchlist = [[x, 0] for x in range(8)]
    branchlist += [[x, 1] for x in range(8)]
    for target in branchlist:
        TARGET = target
        inputvector = [random.randrange(1, 1000) for x in range(3)]
        result = avm_search(target, inputvector)
        if result[1] == 0:
            print(TARGET, result[0])
        else:
            print(TARGET, "fail")


if __name__ == "__main__":
    main()
#TODO: fitness(TARGET, vector), caller of avm_serach(input_vector)
