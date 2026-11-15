#include "metro.h"

#include <algorithm>
#include <cstdlib>

using namespace std;

static const int EXTRA_MINUTES = 120;

static double randomFraction()
{
    return rand() / (RAND_MAX + 1.0);
}

SimResult runSimulation(const SimConfig& cfg, const Array<string>& lineNames)
{
    SimResult result;
    const int lineCount = lineNames.empty() ? 1 : (int)(lineNames.size());
    const int duration = max(1, cfg.durationMin);
    const int freq = max(1, cfg.frequencyMin);
    const int capacity = max(1, cfg.capacity);
    const int platformSize = max(1, cfg.platformCapacity);

    result.passengers = max(0, cfg.passengers);
    result.lines.resize(lineCount);
    for (int l = 0; l < lineCount; l++)
        result.lines[l].name = lineNames.empty() ? "Line" : lineNames[l];

    srand(cfg.seed);
    Array<Array<int>> arrivals(lineCount, Array<int>(duration, 0));
    for (int i = 0; i < result.passengers; i++) {
        double u = (randomFraction() + randomFraction()) / 2.0;
        int minute = min(duration - 1, (int)(u * duration));
        int line = rand() % lineCount;
        arrivals[line][minute]++;
        result.lines[line].arrived++;
    }

    Array<CircularQueue*> platforms(lineCount, nullptr);
    Array<Queue*> gates(lineCount, nullptr);
    bool allocated = true;
    for (int l = 0; l < lineCount; l++) {
        platforms[l] = newCircularQueue((size_t)platformSize);
        gates[l] = newQueue();
        allocated = allocated && platforms[l] != nullptr && gates[l] != nullptr;
    }

    Array<long long> waitSum(lineCount, 0);
    Array<double> occupancySum(lineCount, 0.0);

    int t = 0;
    for (; allocated; t++) {
        bool anyWaiting = false;
        for (int l = 0; l < lineCount; l++) {
            SimLineResult& lr = result.lines[l];

            if (t < duration) {
                for (int k = 0; k < arrivals[l][t]; k++) {
                    if (!(queueIsEmpty(gates[l]) && cqEnqueue(platforms[l], t))) {
                        enqueue(gates[l], t);
                        lr.heldAtGate++;
                    }
                }
            }

            if ((t + l) % freq == 0) {
                int boarded = 0;
                int arrivedAt = 0;
                while (boarded < capacity && cqDequeue(platforms[l], &arrivedAt)) {
                    int wait = t - arrivedAt;
                    waitSum[l] += wait;
                    lr.maxWait = max(lr.maxWait, wait);
                    boarded++;
                }
                lr.served += boarded;
                lr.trains++;
                if (boarded == capacity)
                    lr.fullTrains++;
                occupancySum[l] += 100.0 * boarded / capacity;
            }

            int arrivedAt = 0;
            while (!cqIsFull(platforms[l]) && dequeue(gates[l], &arrivedAt))
                cqEnqueue(platforms[l], arrivedAt);

            lr.maxAtGate = max(lr.maxAtGate, (int)(queueSize(gates[l])));
            if (!cqIsEmpty(platforms[l]) || !queueIsEmpty(gates[l]))
                anyWaiting = true;
        }
        if (t >= duration - 1 && (!anyWaiting || t >= duration + EXTRA_MINUTES))
            break;
    }
    result.minutesSimulated = t + 1;

    long long totalWait = 0;
    double totalOccupancy = 0.0;
    for (int l = 0; l < lineCount; l++) {
        SimLineResult& lr = result.lines[l];
        lr.avgWait = lr.served > 0 ? (double)(waitSum[l]) / lr.served : 0.0;
        lr.avgOccupancy = lr.trains > 0 ? occupancySum[l] / lr.trains : 0.0;
        result.served += lr.served;
        result.trains += lr.trains;
        result.fullTrains += lr.fullTrains;
        result.maxWait = max(result.maxWait, lr.maxWait);
        totalWait += waitSum[l];
        totalOccupancy += occupancySum[l];
        result.heldAtGate += lr.heldAtGate;
        result.maxAtGate = max(result.maxAtGate, lr.maxAtGate);
        result.leftBehind += (int)(cqSize(platforms[l]) + queueSize(gates[l]));
        freeCircularQueue(platforms[l]);
        freeQueue(gates[l]);
    }
    if (!allocated)
        result.leftBehind = result.passengers - result.served;
    result.avgWait = result.served > 0 ? (double)totalWait / result.served : 0.0;
    result.avgOccupancy = result.trains > 0 ? totalOccupancy / result.trains : 0.0;
    return result;
}
