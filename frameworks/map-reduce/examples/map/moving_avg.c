
float arr[5] = {0};

float map(float f) {
    for(int i = 0; i < 4; i++) {
        arr[i] = arr[i+1];
    }

    arr[4] = f;

    float acc = 0;

    for(int i = 0; i < 5; i++) {
        acc += arr[i];
    }

    return acc/5;
}
