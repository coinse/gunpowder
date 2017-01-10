import random
from evaluation import ObjFunc

def avm_search(objfunc, input_vector):
    best = input_vector
    for idx in range(len(input_vector)):
        candidate = iterative_pattern_search(objfunc, best, idx)
        if objfunc.fitness(best) > objfunc.fitness(candidate):
            best = candidate
#        print("AVM", best)
    return [best, objfunc.fitness(best)]


def iterative_pattern_search(objfunc, input_vector, idx):
    SCALING_FACTOR = 2
    STEP = 1
    #TARGET = target


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
            if objfunc.fitness(candidate) < objfunc.fitness(better):
                better = candidate
            else:
                break
#        print("Pattern", best)
        return better

    def search_direction(vector, index):
        left = vector[:]
        left[index] += -1

        right = vector[:]
        right[index] += 1

        if objfunc.fitness(right) < objfunc.fitness(vector):
            return 1
        elif objfunc.fitness(left) < objfunc.fitness(vector):
            return -1
        else:
            return 0

    best = input_vector
    while True:
        candidate = pattern_search(best, idx)
        if objfunc.fitness(best) > objfunc.fitness(candidate):
            best = candidate
        else:
            break
#    print("Iter", best)
    return best


def main():
    branchlist = [[x, 0] for x in range(8)]
    branchlist += [[x, 1] for x in range(8)]
    for target in branchlist:
        objfunc = ObjFunc(target)
        inputvector = [random.randrange(1, 10) for x in range(3)]
        result = avm_search(objfunc, inputvector)
        if result[1] == 0:
            print(target, result[0])
        else:
            print(target, "fail", result[0])


if __name__ == "__main__":
    main()
