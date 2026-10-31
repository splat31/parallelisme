import os

import sys
import subprocess
import pandas as pd
import tempfile

def validate(txt, output):
    res = {}
    for line in output.splitlines():
        try:
            (header, content) = line.split()
        except:
            continue
        if header == b'Duration:':
            try:
                res['Duration'] = float(content)
            except:
                print('Duration not the right format', txt)
                res['Duration'] = None
        elif header == b'Signature:':
            try:
                res['Signature'] = int(content)
            except:
                print('Signature not the right format', txt)
                res['Signature'] = None
        elif header == b'Interactions:':
            try:
                tmp = int(content)
                assert(tmp>=0)
                res['Interactions'] = tmp
            except:
                print('Interactions not the right format', txt)
                res['Interactions'] = None
        elif header == b'Unbalance:':
            try:
                tmp = int(content)
                assert(tmp>=0)
                res['Unbalance'] = tmp
            except:
                print('Unbalance not the right format', txt)
                res['Unbalance'] = None
    if len(res) != 4:
        print('At least one element is missing, I have:', list(res.keys()), txt)
    return res


def get_result(param, sig, nb, verbose=True):
    tests = {}
    for name in 'seq', 'par_masses', 'par_springs', 'par_both', 'par_opti':
        if name == 'seq':
            cmd=f'python3 ./spring_{name}.py {param} -nodisplay'
        else:
            cmd=f'mpirun -n {nb} python3 ./spring_{name}.py {param} -nodisplay'

        try:
            if verbose:
                print('Running', cmd)
            result = subprocess.check_output(cmd, shell=True, stderr=subprocess.DEVNULL)
            tests[name] = validate(cmd, result)
        except:
            if not verbose:
                print('Running', cmd)
            print('  ! Command crashed')
            tests[name] = {'Duration': None, 'Signature':None, 'Interactions':None, 'Unbalance':None}
            continue
        if tests[name]['Signature'] != sig:
            print('  ! Signature error for', name, 'on', param,':',
                  tests[name]['Signature'], 'while it should be', sig)
    return tests

def get_df(raw, target):
    lines = []
    names = None
    for expe in raw:
        current = []
        if names is None:
            names = list(raw[expe].keys())

        lines.append([expe]+[raw[expe][name][target] for name in names])

    a = pd.DataFrame(lines, columns=['nbhosts']+names)
    return(a)


def grade_directory(dirname='.'):
    os.chdir(dirname)

    keywords = ['bcast', 'gather', 'allgather', 'scatter', 'reduce', 'allreduce', 'send', 'recv']
    full = '|\\.'.join(keywords)
    cmd1 = f"grep -r -E '\\.{full}' *"

    try:
        result = subprocess.check_output(cmd1, shell=True, stderr=subprocess.DEVNULL)
    except:
        print("Seems to only use Numpy based mpi, continue")
    else:
        print("Seems to use non-numpy based mpi, current maximum grade is 0")
        print("Faulty lines:")
        print()
        print(result.decode())
        sys.exit(0)

    #param, sig = '300 20100 20', 147
    #param, sig = '12 12 1', -1510
    param, sig = '240 -1 100', -13737

    grade = 0
    res = {}
    for n in 1,2,3,4:
        r = get_result(param, sig, n, verbose=True)
        res[n] = r
        grade += [r[key]['Signature']==sig for key in r].count(True)

    for target in 'Duration', 'Interactions', 'Unbalance':
        df = get_df(res, target)
        print(target)
        df.set_index('nbhosts', inplace=True)
        print(df)
        print()            


    print(f'Maximal final grading {grade}/20')
    print('Depending on the actual code, this value is the maximal grade you can have')


if __name__ == '__main__':

    targetdir = '.'
    if len(sys.argv) == 2:
        target = sys.argv[1]
        if os.path.isdir(target):
            print('Directory can be used for tests but a tar file MUST be provided on Moodle')
            targetdir = target
        else:
            targetdir = tempfile.mkdtemp(prefix='dm_mpi_', dir='.')
            print(f'Target directory: {targetdir}')
            try:
                cmd = f'tar -C {targetdir} -xf {target}'
                result = subprocess.check_output(cmd, shell=True, stderr=subprocess.DEVNULL)
            except:
                print(f'Cannot untar {target}')
                sys.exit(1)
    else:
        print('Directory can be used for tests but a tar file MUST be provided on Moodle')

            

    grade_directory(targetdir)

