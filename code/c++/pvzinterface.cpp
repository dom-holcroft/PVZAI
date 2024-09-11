#include <Python.h>
#include <numpy/arrayobject.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <memoryconfig.h>
#include "pvz.h"

typedef struct
{
    PyObject_HEAD
        HANDLE process;
    MemoryConfig::PlacePlantAddresses placePlantAddresses;
    std::vector<int> seedIDs;
    DWORD rewardAddress;
    int actionInterval;
    double survivalReward;
    DWORD restartFlagAddress;
} PVZProcessObject;

static int
PVZProcess_init(PVZProcessObject *self, PyObject *args)
{
    const char *process_name;
    if (!PyArg_ParseTuple(args, "s", &process_name))
        return -1;

    HANDLE processHandle = getProcessByName(process_name);

    if (processHandle == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to find the process");
        return -1;
    }

    self->process = processHandle;
    return 0;
}

static void
PVZProcess_dealloc(PVZProcessObject *self)
{
    if (self->process)
    {
        CloseHandle(self->process);
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
setup_code_injection(PVZProcessObject *self, PyObject *args)
{
    SetupCodeInjectionReturn setupInjectionReturn = {setupCodeInjection(self->process, &self->placePlantAddresses)};
    self->actionInterval = setupInjectionReturn.actionInterval;
    self->restartFlagAddress = setupInjectionReturn.restartFlagAddress;
    self->survivalReward = setupInjectionReturn.survivalReward;
    self->rewardAddress = setupInjectionReturn.rewardAddress;
    self->seedIDs = setupInjectionReturn.seedIDs;
    Py_RETURN_NONE;
}

static PyObject *
get_game_values(PVZProcessObject *self, PyObject *args)
{

    GameValues gameValues = {getGameValues(self->process, self->restartFlagAddress, self->seedIDs)};

    if(gameValues.gameOver) {
        Py_RETURN_NONE;
    }

    std::vector<int> inputInformation = gameValues.inputInformation;
    std::array<std::array<std::array<int, 10>, 5>, POSSIBLE_ZOMBIES.size() + 10> combinedGrid = gameValues.combinedGrid;

    size_t inputSize = inputInformation.size();
    size_t combinedGridSize = combinedGrid.size() * combinedGrid[0].size() * combinedGrid[0][0].size();


    int *cInputInformation = new int[inputSize];
    int *cCombinedGrid = new int[combinedGridSize];

    if (!cInputInformation || !cCombinedGrid)
    {
        delete[] cCombinedGrid;
        delete[] cInputInformation;
        return PyErr_NoMemory();
    }

    std::copy(inputInformation.begin(), inputInformation.end(), cInputInformation);

    size_t idx = 0;
    for (const auto &layer : combinedGrid)
    {
        for (const auto &row : layer)
        {
            std::copy(row.begin(), row.end(), cCombinedGrid + idx);
            idx += row.size();
        }
    }

    npy_intp combinedGridDims[3] = {POSSIBLE_ZOMBIES.size() + 10, 5, 10};

    PyObject *npCombinedGrid = PyArray_SimpleNewFromData(
        3, combinedGridDims, NPY_INT32, cCombinedGrid);

    if (npCombinedGrid == nullptr)
    {
        delete[] cCombinedGrid;
        delete[] cInputInformation;
        return PyErr_NoMemory();
    }

    PyArray_ENABLEFLAGS(reinterpret_cast<PyArrayObject *>(npCombinedGrid), NPY_ARRAY_OWNDATA);

    npy_intp inputDims[1] = {static_cast<npy_intp>(inputSize)};
    PyObject *npInputInformation = PyArray_SimpleNewFromData(
        1, inputDims, NPY_INT32, cInputInformation);

    if (npInputInformation == nullptr)
    {
        Py_DECREF(npCombinedGrid);
        delete[] cCombinedGrid;
        delete[] cInputInformation;
        return PyErr_NoMemory();
    }

    PyArray_ENABLEFLAGS(reinterpret_cast<PyArrayObject *>(npInputInformation), NPY_ARRAY_OWNDATA);

    

    PyObject *result = PyTuple_New(2);
    if (result == nullptr)
    {
        Py_DECREF(npCombinedGrid);
        Py_DECREF(npInputInformation);
        return PyErr_NoMemory();
    }
    PyTuple_SET_ITEM(result, 0, npCombinedGrid);
    PyTuple_SET_ITEM(result, 1, npInputInformation);

    return result;
}

static PyObject *
play_step(PVZProcessObject *self, PyObject *args)
{
    int previousTime;
    double reward;

    if (!PyArg_ParseTuple(args, "i", &previousTime))
        return NULL;

    int currentTime = playStep(self->process, self->actionInterval, previousTime);
    reward = getReward(self->process, self->rewardAddress, self->survivalReward);
    PyObject *pyReward = PyFloat_FromDouble(reward);

    PyObject *pyCurrentTime = PyLong_FromLong(currentTime);



    PyObject *result = PyTuple_New(2);
    if (result == nullptr)
    {
        Py_DECREF(pyReward);
        Py_DECREF(pyCurrentTime);
        return PyErr_NoMemory();
    }
    PyTuple_SET_ITEM(result, 0, pyCurrentTime);
    PyTuple_SET_ITEM(result, 1, pyReward);

    return result;
}

static PyObject *
restart_game(PVZProcessObject *self, PyObject *args)
{
    restartGame(self->process, self->restartFlagAddress);
    Py_RETURN_NONE;
}

static PyObject *
is_game_running(PVZProcessObject *self, PyObject *args)
{
    return PyBool_FromLong(isGameRunning(self->process));
}

static PyObject *
is_game_over(PVZProcessObject *self, PyObject *args)
{
    return PyBool_FromLong(isGameOver(self->process, self->restartFlagAddress));
}

static PyObject *
place_plant(PVZProcessObject *self, PyObject *args)
{
    int col;
    int row;
    int seedSlot;

    if (!PyArg_ParseTuple(args, "iii", &col, &row, &seedSlot))
        return NULL;
    placePlant(self->process, &self->placePlantAddresses, col, row, seedSlot);
    Py_RETURN_NONE;
}

static PyMethodDef PVZProcess_methods[] = {
    {"setup_code_injection", (PyCFunction)setup_code_injection, METH_VARARGS,
     "Injects some modified code to let the ai interact better"},
    {"place_plant", (PyCFunction)place_plant, METH_VARARGS,
     "Attempts to place a plant on the screen"},
    {"restart_game", (PyCFunction)restart_game, METH_VARARGS,
     "Once a round is done, sets the flag to start another round"},
    {"get_game_values", (PyCFunction)get_game_values, METH_VARARGS,
     "Gets the data from the game for the Neural Network"},
    {"play_step", (PyCFunction)play_step,
     METH_VARARGS, "Play game for action_interval equivelent time"},
    {"is_game_running", (PyCFunction)is_game_running,
     METH_VARARGS, " gets whether the AI is currently in a game"},
    {"is_game_over", (PyCFunction)is_game_over,
     METH_VARARGS, "checks if the restart flag is set to true"},
    {NULL, NULL, 0, NULL}};

static PyTypeObject PVZProcessType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                   .tp_name = "pvz.Process",
    .tp_basicsize = sizeof(PVZProcessObject),
    .tp_dealloc = (destructor)PVZProcess_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Custom objects"),
    .tp_methods = PVZProcess_methods,
    .tp_init = (initproc)PVZProcess_init,
    .tp_new = PyType_GenericNew,
};

static struct PyModuleDef pvzmodule = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "pvzinterface",
    .m_doc = NULL,
    .m_size = -1};

PyMODINIT_FUNC
PyInit_pvzinterface(void)
{

    import_array();

    PyObject *m;
    if (PyType_Ready(&PVZProcessType) < 0)
        return NULL;

    m = PyModule_Create(&pvzmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&PVZProcessType);
    if (PyModule_AddObject(m, "Process", (PyObject *)&PVZProcessType) < 0)
    {
        Py_DECREF(&PVZProcessType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}