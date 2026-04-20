# 🏭 UE5 Factory Logistics Simulation
**대규모 물류 연산 및 렌더링 최적화에 초점을 맞춘 언리얼 엔진 5(UE5) 기반의 공장 시뮬레이션 테크 데모입니다.**

수천 개의 액터가 개별적으로 Tick을 수행하며 발생하는 병목(Bottleneck)을 해결하고자,
서브시스템(Subsystem) 기반의 전역 제어 아키텍처와 위상 정렬(Topological Sort) 알고리즘을 도입하여 대규모 객체 환경에서 프레임 드랍을 최소화하고 안정적인 60 FPS 프레임 유지력을 확보했습니다

## 📌 주요 특징 (Features)
- 6,400개 이상의 벨트 환경에서 **16.6ms(60FPS) 고정 방어**
- `FactoryCycleSubsystem`을 통한 개별 액터 틱 제거 및 전역 연산 일괄 처리
- **위상 정렬(Topological Sort)** 을 이용한 결정론적 물류 적체(Backpressure) 알고리즘
- ISM(Instanced Static Mesh) 기반 렌더링 최적화
- 아이템 속성 및 레시피의 `DataAsset`화를 통한 데이터 주도(Data-Driven) 설계

---

## 🏗 핵심 아키텍처 (Architecture)

대규모 물류 연산 시 액터 틱(Tick) 오버헤드를 막기 위해, 데이터 흐름과 렌더링을 분리한 중앙 집중식 아키텍처를 설계했습니다.

- `UFactoryCycleSubsystem`: **(System Manager)** 맵 내의 모든 물류 객체를 관리하고, 매 프레임 위상 정렬을 통해 연산 순서를 결정합니다.
- `AFactoryLogisticsObjectBase`: **(Logistics Entity)** 개별 틱 연산을 하지 않고, 서브시스템의 호출(`InitPhase`, `LogisticsPhase` 등)에 따라 상태만 갱신합니다.
- `AFactoryItemRenderActor`: **(Rendering Agent)** 객체들의 트랜스폼(Transform) 데이터를 받아 **ISM(Instanced Static Mesh)** 기반으로 드로우 콜(Draw Call)을 묶어 일괄 렌더링합니다.
- `UPrimaryDataAsset`: **(Data Definition)** 설비 속성, 레시피, 포트 위치 등을 데이터화하여 하드코딩 배제 및 확장성 확보

---

## 🚀 성능 최적화 리포트 (Performance Profiling)
6,400개의 컨베이어 벨트 및 풀링된 아이템 환경에서 `Unreal Insights`를 활용하여 병목을 추적하고 최적화를 진행했습니다.

> <img width="2560" height="1440" alt="image" src="https://github.com/user-attachments/assets/b1fc0f09-9d3a-4a9b-ae56-e0595ff0249d" />
[Profiling: Test Environment]

### 1. Baseline: 개별 Actor Tick 사용
- **현상:** GameThread 내부 `UWorld_Tick` 연산에 약 16~18ms 소요. 간헐적 스파이크 발생(최대 30ms).
- **진단:** 개별 액터의 충돌 및 이동 연산으로 인한 막대한 Tick 오버헤드 발생.

> <img width="1660" height="955" alt="image" src="https://github.com/user-attachments/assets/e1041519-f300-4ad8-95c1-b1025f319735" />
[Profiling: Baseline Stage]

### 2. Opt 1: Subsystem Tick Control 도입
- **조치:** 벨트 액터의 Tick을 비활성화하고 매니저를 통해 물류 업데이트 로직 일원화.
- **결과:** `UWorld_Tick`이 약 8.5~10ms로 40% 이상 감소.

> <img width="1659" height="952" alt="image" src="https://github.com/user-attachments/assets/6553ac78-1ea7-4628-bbe2-5fad501874fe" />
[Profiling: Opt 1 Stage]

### 3. Opt 2: Caching & Rendering 최적화 (최종)
- **조치:** 스플라인 보간 연산 부하를 줄이기 위해 생성 시점에 트랜스폼 데이터를 배열로 **Pre-computing(Caching)** 처리. 불필요한 그림자 연산 및 루먼(Lumen) 부하 억제 테스트 병행.
- **결과:** `UWorld_Tick` **약 3.5~5ms 수준으로 최적화. Baseline 대비 약 80% 성능 개선**.
- **회고 및 향후 과제:** 현재 AActor 기반 시스템의 한계를 인지하였으며, 향후 **Lightweight Struct(ECS-like)** 기반 데이터 구조로 전환하고 렌더링은 `HISM`으로 완전히 분리하는 아키텍처 개편안을 도출함.

> <img width="1655" height="956" alt="image" src="https://github.com/user-attachments/assets/961a032f-3e2d-4f77-aca7-d87f265671c7" />
[Profiling: Opt 2 Stage]

---

## ⚙️ 핵심 알고리즘: 순환 예외 처리가 포함된 위상 정렬
단일 벨트 액터의 매 프레임 충돌 검사($O(N^2)$) 오버헤드를 탈피하기 위해, `FactoryCycleSubsystem`에서 전체 물류 네트워크를 **방향성 비순환 그래프(DAG)**로 강제하여 위상 정렬을 수행합니다. 

특히 유저의 임의 배치로 인해 논리적 순환(Cycle) 궤도가 생성되어 알고리즘이 데드락에 빠지는 상황을 감지하고, `CycleBreaker` 로직으로 순환의 고리를 끊어내어 런타임 안정성과 결정론적 흐름(Deterministic Order)을 완벽히 보장합니다.

```cpp
void UFactoryCycleSubsystem::SortRegisteredLogisticsObjectArr()
{
    TMap<AFactoryLogisticsObjectBase*, int32> OutportConnectedMap;
    TQueue<AFactoryLogisticsObjectBase*> ZeroOutportQueue;
    TArray<TWeakObjectPtr<AFactoryLogisticsObjectBase>> SortedOutportArray;
    
    // 1. 초기화: 연결된 Outport가 없는 설비를 Queue에 삽입
    for (int i = 0; i < RegisteredLogisticsObjectArr.Num(); i++)
    {
       if (AFactoryLogisticsObjectBase* RegisteredObjPtr = RegisteredLogisticsObjectArr[i].Get())
       {
          int32 ConnectedOutputPortNumber = RegisteredObjPtr->GetConnectedOutputPortNumber();
          if (ConnectedOutputPortNumber == 0) ZeroOutportQueue.Enqueue(RegisteredObjPtr);
          else OutportConnectedMap.Add(RegisteredObjPtr, ConnectedOutputPortNumber);
       }
    }
    
    // 2. Kahn's Algorithm 수행 (SafetyCounter로 무한루프 방어)
    int32 SafetyCounter = 0;
    const int32 MaxIterations = RegisteredLogisticsObjectArr.Num() * 2;
    
    while (SortedOutportArray.Num() < RegisteredLogisticsObjectArr.Num() && SafetyCounter < MaxIterations)
    {
       SafetyCounter++;
       AFactoryLogisticsObjectBase* DequeuedObject = nullptr;

       if (ZeroOutportQueue.Dequeue(DequeuedObject))
       {
          TArray<AFactoryLogisticsObjectBase*> ConnectedObject = DequeuedObject->GetConnectedInputPortsObject();
          for (int i = 0; i < ConnectedObject.Num(); i++)
          {
             if (int32* OutDegreePtr = OutportConnectedMap.Find(ConnectedObject[i]))
             {
                (*OutDegreePtr)--; // 간선 제거
                if (*OutDegreePtr <= 0)
                {
                   ZeroOutportQueue.Enqueue(ConnectedObject[i]);
                   OutportConnectedMap.Remove(ConnectedObject[i]);
                }
             }
          }
          SortedOutportArray.Add(DequeuedObject);
       }
       else
       {
          // [Exception Handling] 순환 참조 시 데드락 방지 로직 (CycleBreaker)
          AFactoryLogisticsObjectBase* CycleBreaker = nullptr;
          for (auto& Pair : OutportConnectedMap) { CycleBreaker = Pair.Key; break; }
          
          if (CycleBreaker)
          {
             ZeroOutportQueue.Enqueue(CycleBreaker);
             OutportConnectedMap.Remove(CycleBreaker);
          }
       }
    }
    
    // 3. 정렬된 배열 캐싱 및 적용
    // (만약 SafetyCounter가 터졌더라도, 남은 객체들을 증발시키지 않고 보존)
    for (auto& Pair : OutportConnectedMap) SortedOutportArray.Add(Pair.Key);
    RegisteredLogisticsObjectArr = SortedOutportArray;
}
```

---

## 🔧 개발환경
* Engine: Unreal Engine 5.6
* IDE : JetBrains Rider 2025.3.2

---

## ⚠️ Disclaimer (저작권 안내)
* 본 프로젝트는 개인의 기술 포트폴리오 및 비상업적 목적을 위해 제작된 테크 데모입니다.
* 프로젝트에 사용된 일부 UI 이미지 에셋은 '명일방주: 엔드필드(Arknights: Endfield)'의 에셋을 모작 및 임시 차용하였으며, 해당 에셋의 모든 저작권은 원저작자(Hypergryph)에게 있습니다.
* 그래픽 에셋을 제외한 모든 시스템 구조 설계, C++ 로직 구현 및 최적화 코드는 본인이 직접 작성하였습니다.

---

## 📜 License
* **Source Code:** 이 레포지토리의 소스 코드(C++)는 `MIT License`를 따릅니다.
* **Assets:** 프로젝트에 사용된 일부 UI 이미지(명일방주: 엔드필드 차용) 및 그래픽 에셋은 원저작자에게 권리가 있으며, 무단 배포 및 상업적 사용을 금지합니다.
