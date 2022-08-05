#pragma once
#include "common.h"

// �ڴ�������͹�ϣͰ�±����
class Utility
{
private:
	static size_t RoundUpHelper(size_t bytes, int alignNum);

	static size_t IndexHelper(size_t bytes, size_t alignShift);
public:
	// ��size���϶���
	static size_t RoundUp(size_t bytes);
	  
	// ��ȡ������СΪsize���ڴ���ڹ�ϣ���е��±�
	static int Index(size_t size);

	static size_t NumMoveSize(size_t size);

	// ����size������Ӧ����npageҳ��Span�ָ����
	static size_t NumMovePage(size_t size);
};