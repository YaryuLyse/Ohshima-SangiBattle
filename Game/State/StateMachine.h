#pragma once

template<typename OwnerType>
class StateMachine
{
public:
    void Initialize(OwnerType* pOwner) 
    {
        m_pOwner = pOwner;
    }

    void ChangeState(StateBase<OwnerType>* pNewState) 
    {
        if (m_pCurrentState) 
        {
            m_pCurrentState->CallExit(m_pOwner);
        }

        m_pCurrentState = pNewState;

        if (m_pCurrentState) 
        {
            m_pCurrentState->CallStart(m_pOwner);
        }
    }

    void Update() 
    {
        if (m_pCurrentState) 
        {
            m_pCurrentState->CallUpdate(m_pOwner);
        }
    }
private:
	StateBase<OwnerType>* m_pCurrentState = nullptr; // 現在のステート
	OwnerType* m_pOwner = nullptr; // ステートマシンの持ち主
    
};

