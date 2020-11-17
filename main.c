#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

const unsigned int size = 100; unsigned long time1,time2;

// to open the file and check it
void openFile(FILE *file,char *path) {
    file = freopen(path,"r",stdin);
    if (file == NULL) {
        printf("File %s is not found\n",path);
        exit(1);
    }
}
// to close the file
void closeFile(FILE *file) {
    fclose(stdin);
}
/*
 * takes the first line in the input file and return number
 */
int get() {

    char str[size]; // i assume the max number of digits the user will enter not exceed 6
    scanf("%s",str);
    int res = 0;
    for (int i = 4; *(str+i) != '\0' ; ++i) {
        res *= 10; res += ( (char)(*(str+i)) - '0');
    }
    return res;

}

/*
 * read the files and build the matrix
 */
int ** getMat(int *r,int *c,FILE *file,char *path) {
    openFile(file,path);

    *r = get(); // number of rows
    *c = get(); // number of cols
    int ** mat = (int **)malloc(sizeof(int *)*(*r));
    for (int i = 0; i < *r; ++i) {
        mat[i] = (int *)malloc(sizeof(int)*(*c));
        for (int j = 0; j < *c; ++j) {
            scanf("%d",&mat[i][j]);
        }
    }
    closeFile(file);
    return mat;
}
// for printing the matrix
void printMat(int **mat,int x, int y) {

    for (int i = 0; i < x; ++i) {
        for (int j = 0; j < y; ++j) {
            printf("%-20d",mat[i][j]);
        }
        printf("\n");
    }

}

/*
 * work on method one
 */

// stores the data used in function
struct data1{
    int **b; // the whole mat2
    int *a; // the row in mat1
    int *c; // the row in result mat
    int num_row; // the number of row
    int num_col; // the number of col
};

// the function which passed to the thread
void * fun1(void * thread) {

    struct data1 *data = (struct data1 *)thread;
    int *a = data->a;
    int **b = data->b;
    int *c = data->c;
    int row = data->num_row;
    int col = data->num_col;

    for (int i = 0; i < col; ++i) {
        c[i] = 0;
        for (int j = 0; j < row; ++j) {
            c[i] += a[j]*b[j][i];
        }
    }

    pthread_exit(0);
}

void method1(int x,int y,int z,int ** mat1, int ** mat2) {
    struct timeval stop, start; // to calculate the time which the function will take

    int num_thread = x;

    // stores size for result mat
    int **res = (int **)malloc(sizeof(int *)*x);
    for (int i = 0; i < x; ++i) {
        res[i] = (int *)malloc(sizeof(int)*z);
    }

    gettimeofday(&start, NULL); // start time

    pthread_t threads[num_thread]; struct data1 *data1[num_thread]; // to store the struct which still in memory to clean up it

    for (int i = 0; i < num_thread; ++i) {

        struct data1 *data = malloc(sizeof(struct data1));
        data->a = mat1[i];
        data->b = mat2;
        data->c = res[i];
        data->num_row = y;
        data->num_col = z;
        int error = pthread_create(&threads[i],NULL,fun1,(void *) data);
        if (error) {
            printf("ERROR; return code from pthread_create() is %d\n", error);
            exit(-1);
        }
        data1[i] = data;
    }

    for (int i = 0; i < num_thread; ++i) {
        pthread_join(threads[i],NULL); // wait for all threads to finish
        free(data1[i]); // clean space form finished threads
    }

    gettimeofday(&stop, NULL); // end time

    printf("Method1 result\n\n");
    printMat(res,x,z);
    printf("\n\n");

    free(res);

    time1 = stop.tv_usec - start.tv_usec;
}

/*
 * work on method two
 */

struct data2{
    int **b; // the whole matrix two
    int *a; // the row in matrix one
    int *c; // element in result matrix
    int num_row; // the number of row
    int num_col; // the column number for matrix two
};

// function used in the thread
void * fun2(void * thread) {

    struct data2 *data = (struct data2 *)thread;
    int *a = data->a;
    int **b = data->b;
    int *c = data->c;
    int row = data->num_row;
    int index = data->num_col;
    *c = 0; // make the element with zero in the marix
    for (int i = 0; i < row; ++i) {
        *c += a[i]*b[i][index];
    }

    pthread_exit(0);
}

void method2(int x, int y, int z, int ** mat1, int ** mat2) {
    struct timeval stop, start;

    int **res = (int **)malloc(sizeof(int *)*x);
    for (int i = 0; i < x; ++i) {
        res[i] = (int *)malloc(sizeof(int)*z);
    }

    pthread_t threads[x][z]; struct data2 *data2[x][z];

    gettimeofday(&start, NULL);

    for (int i = 0; i < x; ++i) {
        for (int j = 0; j < z; ++j) {
            struct data2 *data = malloc(sizeof(struct data2));

            data->a = mat1[i];
            data->b = mat2;
            data->c = &res[i][j];
            data->num_row = y;
            data->num_col = j;

            int error = pthread_create(&threads[i][j],NULL,fun2,(void *) data);
            if (error) {
                printf("ERROR; return code from pthread_create() is %d\n", error);
                exit(-1);
            }

            data2[i][j] = data;
        }
    }

    for (int i = 0; i < x; ++i) {
        for (int j = 0; j < z; ++j) {
            pthread_join(threads[i][j],NULL);
            free(data2[i][j]);
        }
    }

    gettimeofday(&stop, NULL);

    printf("Method2 result\n\n");
    printMat(res,x,z);

    free(res);
    time2 = stop.tv_usec - start.tv_usec;
}

int main(int argc, char **argv) {

    char *input[3] = {"a.txt","b.txt","c.out"}; // the default input and output files

    int x,y1,y2,z; // row and col for mat one, row and col for mat two
    FILE *file; // two read or write to the files input and output
    int old = dup(1); // to store the default stdout

    for (int i = 1; i < argc && i < 4; ++i) {
        input[i-1] = argv[i];
    }

    // create the first mat from first file
    int ** mat1 = (int **)getMat(&x,&y1,file,input[0]);
    // create the second mat from second file
    int ** mat2 = (int **)getMat(&y2,&z,file,input[1]);

    // if the col of mat one not equal the row of mat two
    if (y1 != y2) {
        printf("Error: # cols of mat1 isn't equal to # rows mat2\n");
        exit(1);
    }


    freopen(input[2],"w",stdout);
    // solve with method one
    method1(x,y1,z,mat1,mat2);

    // solve with method two
    method2(x,y1,z,mat1,mat2);

    // return to default stdout
    FILE *fp2 = fdopen(old, "w");
    fclose(stdout);
    stdout = fp2;

    printf("Method one:\n");
    printf("Microseconds taken: %lu\n",time1);
    printf("Threads used: %d\n\n",x);

    printf("Method two:\n");
    printf("Microseconds taken: %lu\n",time2);
    printf("Threads used: %d\n",x*z);

    return 0;
}