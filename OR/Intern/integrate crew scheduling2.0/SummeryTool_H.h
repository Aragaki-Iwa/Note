//ͷ�ļ�����<windows.h>
#include <windows.h>
#ifndef SummeryTool_H
#define SummeryTool_H

namespace Summery
{
	//����QueryPerformanceFrequency��QueryPerformanceCounter
	//Ĭ��Ϊ΢��
	class StopWatch
	{
	public:
		StopWatch() : elapsed(0)
		{
			QueryPerformanceFrequency(&freq);
		}
		~StopWatch(){}

		void Start()
		{
			QueryPerformanceCounter(&begin_time);
		}
		void Stop()
		{
			LARGE_INTEGER end_time;
			QueryPerformanceCounter(&end_time);
			elapsed += (end_time.QuadPart - begin_time.QuadPart) * 1000000 / freq.QuadPart;
		}
		void Restart()
		{
			elapsed = 0;
			Start();
		}

		//΢��
		double Elapsed()
		{
			return static_cast<double>(elapsed);
		}
		//����
		double Elapsed_ms()
		{
			return elapsed / 1000.0;
		}
		//��
		double Elapsed_s()
		{
			return elapsed / 1000000.0;
		}

	private:
		LARGE_INTEGER freq;
		LARGE_INTEGER begin_time;
		long long elapsed;

	};
}
#endif // !SummeryTool_H

