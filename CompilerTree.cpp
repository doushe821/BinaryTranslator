#include "CompilerTree.h"

#include <assert.h>

static void PrintDescendants(const void* Node, FILE* Out);

void* NewNode(const void* Tree, const size_t Type, const size_t DataSize, const void* Value, const size_t Degree, ...)
{
    assert(Tree);

    void* NewNode = calloc(1, ATL_BASE_NODE_SIZE + DataSize + Degree * sizeof(void*));

    size_t BytesFilled = 0;

    memcpy((char*)NewNode, Tree, sizeof(void*));
    BytesFilled += sizeof(Tree);

    memcpy((char*)NewNode + BytesFilled, &Type, sizeof(Type));
    BytesFilled += sizeof(Type);

    memcpy((char*)NewNode + BytesFilled, &DataSize, sizeof(DataSize));
    BytesFilled += sizeof(DataSize);

    if(Value == NULL)
    {
        memset((char*)NewNode + BytesFilled, 0, DataSize);
        BytesFilled += DataSize;
    }
    else
    {
        memcpy((char*)NewNode + BytesFilled, Value, DataSize);
        BytesFilled += DataSize;
    }

    memcpy((char*)NewNode + BytesFilled, &Degree, sizeof(Degree));
    BytesFilled += sizeof(Degree);

    va_list Descendants;
    va_start(Descendants, Degree);
    for(size_t i = 0; i < Degree; i++)
    {
        void* Descendant = va_arg(Descendants, void*);
        if(Descendant == NULL)
        {
            memset((char*)NewNode + BytesFilled, 0, sizeof(Descendant));
            BytesFilled += sizeof(Descendant);
        }
        else
        {
            memcpy((char*)NewNode + BytesFilled, &Descendant, sizeof(Descendant));
            BytesFilled += sizeof(Descendant);
        }
    }

    return NewNode;
}

void* GetNodeData(const void* Node, const int FieldCode, const size_t DescendantNumber)
{
    if(Node == NULL)
    {
        return NULL;
    }

    if(FieldCode == DESCENDANTS_FIELD_CODE)
    {
        return (char*)Node + CalculateByteShift(Node, FieldCode) + sizeof(void*) * DescendantNumber;
    }
    return (char*)Node + CalculateByteShift(Node, FieldCode);
}

int AddDescendant(void* Node, const void* Descendant, size_t DescendantNumber)
{
    assert(memcpy((char*)Node + CalculateByteShift(Node, DESCENDANTS_FIELD_CODE) + sizeof(void*) * DescendantNumber, &Descendant, sizeof(void*))); // TODO remove assert on release FIXME please don't forget                                                                                                                       that you are a precious little gay boy
    return 0;
}

int NodeDump(const void* Node, FILE* Out)
{
    assert(Out);
    assert(Node);
    size_t NodeType = *(size_t*)((char*)Node + CalculateByteShift(Node, TYPE_FIELD_CODE));

    fprintf(Out, "\"label\" = \"{<adr> Node Address =  %p|<value> ", Node);

    switch(NodeType)
    {
        case SYSTEM_FUNCTION_CALL_NODE:
        {
            int Index = 0;
            memcpy(&Index, GetNodeData(Node, DATA_FIELD_CODE, 0), sizeof(Index));
            fprintf(Out, "%s|", kIR_SYS_CALL_ARRAY[Index].Name);
            PrintDescendants(Node, Out);
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"#FAE7B5\"\n");
            break;    
        }
        case RETURN_NODE:
        {
            fprintf(Out, "Return|");
            PrintDescendants(Node, Out);
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"#FAE7B5\"\n");
            break;          
        }
        case FUNCTION_CALL_NODE:
        {
            int Value = 0;
            memcpy(&Value, GetNodeData(Node, DATA_FIELD_CODE, 0), sizeof(Value));
            fprintf(Out, "Function index: %d|Function call node", Value);
            PrintDescendants(Node, Out);
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"cyan\"\n");
            break;          
        }
        case PROGRAM_NODE:
        {
            fprintf(Out, "Program");
            PrintDescendants(Node, Out);
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"#2F4F4F\"\n");
            break;     
        }
        case FUNCTION_ARGUMENTS_NODE:
        {
            int Value = 0;
            memcpy(&Value, GetNodeData(Node, DATA_FIELD_CODE, 0), sizeof(Value));
            fprintf(Out, "Number of qrguments: %zu|Function arguments", *(size_t*)GetNodeData(Node, DATA_FIELD_CODE, 0));
            PrintDescendants(Node, Out);
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"cyan\"\n");
            break;   
        }
        case CONDITION_NODE:
        {
            int Value = 0;
            memcpy(&Value, GetNodeData(Node, DATA_FIELD_CODE, 0), sizeof(Value));
            fprintf(Out, "\\\"%s\\\"|Conditional cycle", KeyWordsArray[Value].Name);
            PrintDescendants(Node, Out);
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"cyan\"\n");
            break;
        }
        case STATEMENT_OPERAND_NODE:
        {
            size_t Value = 0;
            memcpy(&Value, GetNodeData(Node, DATA_FIELD_CODE, 0), sizeof(Value));
            fprintf(Out, "\\\"%s\\\"|Statement operand", KeyWordsArray[Value].Name);
            PrintDescendants(Node, Out);
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"grey\"\n");
            break;
        }
        case SCOPE_NODE:
        {
            size_t Value = 0;
            memcpy(&Value, GetNodeData(Node, DATA_FIELD_CODE, 0), sizeof(Value));
            fprintf(Out, "Number of Statements = %zu|Scope", Value);
            PrintDescendants(Node, Out);
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"purple\"\n");
            break;
        }
        case FUNCTION_BODY_NODE:
        {
            size_t Value = 0;
            memcpy(&Value, GetNodeData(Node, DATA_FIELD_CODE, 0), sizeof(Value));
            fprintf(Out, "Table Number = %zu|Function body", Value);
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"red\"\n");
            break;
        }
        case LEFT_VARIABLE_NODE:
        {
            int Value = 0;
            memcpy(&Value, GetNodeData(Node, DATA_FIELD_CODE, 0), sizeof(Value));
            char Name[VARIABLE_NAME_MAX] = {};
            memcpy(Name, (char*)GetNodeData(Node, DATA_FIELD_CODE, 0) + sizeof(Value), VARIABLE_NAME_MAX);
            fprintf(Out, "Table Number = %d|Name: \\\"%s\\\"|Left Variable", Value, Name);
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"cyan\"\n");
            break;
        }
        case INTEGER_VALUE_NODE:
        {
            int Value = 0;
            memcpy(&Value, GetNodeData(Node, DATA_FIELD_CODE, 0), sizeof(Value));
            fprintf(Out, "%d|Integer Value", Value);
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"cyan\"\n");
            break;
        }
        case FLOAT_VALUE_NODE:
        {
            double Value = 0;
            memcpy(&Value, GetNodeData(Node, DATA_FIELD_CODE, 0), sizeof(Value));
            fprintf(Out, "%lf|Floating Point Value", Value);
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"cyan\"\n");
            break;
        }
        case OPERATION_NODE:
        {
            int OperationCode = 0;
            memcpy(&OperationCode, GetNodeData(Node, DATA_FIELD_CODE, 0), sizeof(OperationCode));
            fprintf(Out, "\\\"\\%s\\\"|Operator", KeyWordsArray[OperationCode].Name);

            PrintDescendants(Node, Out);
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"cyan\"\n");
            break;
        }
        case RIGHT_VARIABLE_NODE:
        {
            fprintf(Out, "%d|Right Variable", *(int*)GetNodeData(Node, DATA_FIELD_CODE, 0));
            fprintf(Out, "}\"\ncolor=\"black\"\nfillcolor=\"cyan\"\n");
            break;
        }
        default:
        {
            fprintf(Out, "Zamn, an error\n(What the sigma!?)\n");
            break;
        }
    }

    fprintf(Out, "shape = \"record\"];");
    return 0;
}

static void PrintDescendants(const void* Node, FILE* Out)
{
    assert(Node);
    assert(Out);

    size_t Degree = 0;
    memcpy(&Degree, GetNodeData(Node, DEGREE_FIELD_CODE, 0), sizeof(Degree));

    if(Degree)
    {
        fprintf(Out, "|{");
    }
    for(size_t i = 0; i < Degree; i++)
    {
        char* addr = {};
        memcpy(&addr, GetNodeData(Node, DESCENDANTS_FIELD_CODE, i), sizeof(void*));
        if(i != Degree - 1)
        {
            fprintf(Out, "<d%zu>%p |", i, addr);
        }
        else
        {
            fprintf(Out, "<d%zu>%p}", i, addr);
        }
    }
}


// TODO (if needed)

void* CloneTree(const void* tree, void* root)
{
    return (void*)((size_t)tree + (size_t)root);
}
