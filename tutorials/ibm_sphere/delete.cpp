#include <vtkSmartPointer.h>
#include <vtkExodusIIReader.h>
#include <vtkSTLReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkExodusIIWriter.h>
#include <vtkCompositeDataIterator.h>

// Function to delete enclosed elements in a mesh based on a surface
vtkSmartPointer<vtkUnstructuredGrid> deleteEnclosedElements(vtkSmartPointer<vtkUnstructuredGrid> mesh, vtkSmartPointer<vtkPolyData> surface) {
    vtkSmartPointer<vtkImplicitPolyDataDistance> implicitFunction = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
    implicitFunction->SetInput(surface);

    vtkSmartPointer<vtkCellArray> newCells = vtkSmartPointer<vtkCellArray>::New();

    for (vtkIdType cellId = 0; cellId < mesh->GetNumberOfCells(); ++cellId) {
        vtkCell* cell = mesh->GetCell(cellId);
        vtkPoints* points = cell->GetPoints();
        bool allInside = true;

        for (vtkIdType pointId = 0; pointId < points->GetNumberOfPoints(); ++pointId) {
            double p[3];
            points->GetPoint(pointId, p);
            if (implicitFunction->EvaluateFunction(p) >= 0) {
                allInside = false;
                break;
            }
        }

        if (!allInside) {
            newCells->InsertNextCell(cell);
        }
    }

    vtkSmartPointer<vtkUnstructuredGrid> newMesh = vtkSmartPointer<vtkUnstructuredGrid>::New();
    newMesh->SetPoints(mesh->GetPoints());
    newMesh->SetCells(mesh->GetCellTypesArray(), mesh->GetCellLocationsArray(), newCells);

    newMesh->GetPointData()->ShallowCopy(mesh->GetPointData());
    newMesh->GetCellData()->ShallowCopy(mesh->GetCellData());

    return newMesh;
}

// Function to copy side sets from the source mesh to the target mesh
void CopySideSets(vtkSmartPointer<vtkUnstructuredGrid> sourceMesh, vtkSmartPointer<vtkUnstructuredGrid> targetMesh) {
    vtkPointData* sourcePointData = sourceMesh->GetPointData();
    vtkPointData* targetPointData = targetMesh->GetPointData();

    for (int i = 0; i < sourcePointData->GetNumberOfArrays(); ++i) {
        vtkDataArray* dataArray = sourcePointData->GetArray(i);
        if (dataArray) {
            vtkSmartPointer<vtkDataArray> copiedArray = vtkSmartPointer<vtkDataArray>::NewInstance(dataArray);
            copiedArray->DeepCopy(dataArray);
            targetPointData->AddArray(copiedArray);
        }
    }

    vtkCellData* sourceCellData = sourceMesh->GetCellData();
    vtkCellData* targetCellData = targetMesh->GetCellData();

    for (int i = 0; i < sourceCellData->GetNumberOfArrays(); ++i) {
        vtkDataArray* dataArray = sourceCellData->GetArray(i);
        if (dataArray) {
            vtkSmartPointer<vtkDataArray> copiedArray = vtkSmartPointer<vtkDataArray>::NewInstance(dataArray);
            copiedArray->DeepCopy(dataArray);
            targetCellData->AddArray(copiedArray);
        }
    }
}

// Function to process a multi-block dataset and delete enclosed elements from each block
vtkSmartPointer<vtkMultiBlockDataSet> ProcessMultiBlockDataSet(vtkSmartPointer<vtkMultiBlockDataSet> multiBlock, vtkSmartPointer<vtkPolyData> surface) {
    vtkSmartPointer<vtkMultiBlockDataSet> resultMultiBlock = vtkSmartPointer<vtkMultiBlockDataSet>::New();
    resultMultiBlock->CopyStructure(multiBlock);  // Keep the same structure as the original multi-block

    vtkCompositeDataIterator* iter = multiBlock->NewIterator();
    iter->GoToFirstItem();

    while (!iter->IsDoneWithTraversal()) {
        vtkUnstructuredGrid* block = vtkUnstructuredGrid::SafeDownCast(iter->GetCurrentDataObject());
        if (block) {
            vtkSmartPointer<vtkUnstructuredGrid> newMesh = deleteEnclosedElements(block, surface);
            CopySideSets(block, newMesh);  // Copy side sets
            resultMultiBlock->SetDataSet(iter, newMesh);
        }
        iter->GoToNextItem();
    }

    iter->Delete();
    return resultMultiBlock;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <mesh.exo> <surface.stl>" << std::endl;
        return EXIT_FAILURE;
    }

    vtkSmartPointer<vtkExodusIIReader> meshReader = vtkSmartPointer<vtkExodusIIReader>::New();
    meshReader->SetFileName(argv[1]);
    meshReader->Update();
    vtkSmartPointer<vtkMultiBlockDataSet> multiBlock = meshReader->GetOutput();

    vtkSmartPointer<vtkSTLReader> surfaceReader = vtkSmartPointer<vtkSTLReader>::New();
    surfaceReader->SetFileName(argv[2]);
    surfaceReader->Update();
    vtkSmartPointer<vtkPolyData> surface = surfaceReader->GetOutput();

    vtkSmartPointer<vtkMultiBlockDataSet> newMultiBlock = ProcessMultiBlockDataSet(multiBlock, surface);

    vtkSmartPointer<vtkExodusIIWriter> writer = vtkSmartPointer<vtkExodusIIWriter>::New();
    writer->SetFileName("filtered_mesh.exo");
    writer->SetInputData(newMultiBlock);
    writer->Write();

    return EXIT_SUCCESS;
}
